use globals::{
    SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE, SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION,
};
use itf_components::compartment::Compartment;
use windows::{
    core::HRESULT,
    Win32::{
        System::Com::{CoCreateInstance, CLSCTX_INPROC_SERVER},
        UI::TextServices::{
            CLSID_TF_ThreadMgr, ITfThreadMgr, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION,
            GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
            TF_CONVERSIONMODE_FULLSHAPE, TF_CONVERSIONMODE_NATIVE, TF_CONVERSIONMODE_SYMBOL,
        },
    },
};

pub struct CompartmentUpdateListener {
    compartment: Compartment,
    tf_client_id: u32,
}

impl CompartmentUpdateListener {
    pub fn new(thread_mgr: ITfThreadMgr, tf_client_id: u32) -> CompartmentUpdateListener {
        CompartmentUpdateListener {
            compartment: Compartment::new(
                thread_mgr,
                tf_client_id,
                GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION,
            ),
            tf_client_id,
        }
    }

    pub fn init(&self, thread_mgr: ITfThreadMgr, tf_client_id: u32) -> windows::core::Result<()> {
        let keyboard_open = Compartment::new(
            thread_mgr.clone(),
            tf_client_id,
            GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
        );
        keyboard_open.set_bool(true).ok();

        let double_single_byte = Compartment::new(
            thread_mgr.clone(),
            tf_client_id,
            SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE,
        );
        double_single_byte.set_bool(false).ok();

        let punctuation = Compartment::new(
            thread_mgr,
            tf_client_id,
            SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION,
        );
        punctuation.set_bool(true).ok();

        // This seems to be intended, but in the original code this was no-op because of a wrong initialization order.
        // Specifically, _pCompartmentConversion was nullptr at this point.
        // Skipping this as this project wants to keep the original behavior as much as possible.
        // (Uncommenting this will make wide punctuation the default behavior, as set above.)
        // self.private_compartments_updated(thread_mgr);

        Ok(())
    }

    fn toggle_compartment(
        conversion_mode: u32,
        thread_mgr: ITfThreadMgr,
        tf_client_id: u32,
        guid: windows::core::GUID,
        tf_flag: u32,
    ) -> windows::core::Result<()> {
        let compartment = Compartment::new(thread_mgr, tf_client_id, guid);
        let value = compartment.get_bool()?;
        if !value && (conversion_mode & tf_flag != 0) {
            compartment.set_bool(true)?;
        } else if value && (conversion_mode & tf_flag == 0) {
            compartment.set_bool(false)?;
        }
        Ok(())
    }

    pub fn conversion_mode_compartment_updated(&self, thread_mgr: ITfThreadMgr) {
        let conversion_mode = self.compartment.get_u32();
        if conversion_mode.is_err() {
            return;
        }
        let conversion_mode = conversion_mode.unwrap();

        Self::toggle_compartment(
            conversion_mode,
            thread_mgr.clone(),
            self.tf_client_id,
            SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE,
            TF_CONVERSIONMODE_FULLSHAPE,
        )
        .ok();

        Self::toggle_compartment(
            conversion_mode,
            thread_mgr.clone(),
            self.tf_client_id,
            SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION,
            TF_CONVERSIONMODE_SYMBOL,
        )
        .ok();

        Self::toggle_compartment(
            conversion_mode,
            thread_mgr,
            self.tf_client_id,
            GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
            TF_CONVERSIONMODE_NATIVE,
        )
        .ok();
    }

    fn updated_conversion_mode(
        conversion_mode: u32,
        thread_mgr: ITfThreadMgr,
        tf_client_id: u32,
        guid: windows::core::GUID,
        tf_flag: u32,
    ) -> windows::core::Result<u32> {
        let compartment = Compartment::new(thread_mgr, tf_client_id, guid);
        let value = compartment.get_bool()?;
        if !value && (conversion_mode & tf_flag != 0) {
            Ok(conversion_mode & !tf_flag)
        } else if value && (conversion_mode & tf_flag == 0) {
            Ok(conversion_mode | tf_flag)
        } else {
            Ok(conversion_mode)
        }
    }

    fn private_compartments_updated(&self, thread_mgr: ITfThreadMgr) {
        let conversion_mode = self.compartment.get_u32();
        if conversion_mode.is_err() {
            return;
        }
        let mut conversion_mode = conversion_mode.unwrap();
        let conversion_mode_prev = conversion_mode;

        conversion_mode = Self::updated_conversion_mode(
            conversion_mode,
            thread_mgr.clone(),
            self.tf_client_id,
            SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE,
            TF_CONVERSIONMODE_FULLSHAPE,
        )
        .unwrap_or(conversion_mode);

        conversion_mode = Self::updated_conversion_mode(
            conversion_mode,
            thread_mgr,
            self.tf_client_id,
            SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION,
            TF_CONVERSIONMODE_SYMBOL,
        )
        .unwrap_or(conversion_mode);

        if conversion_mode != conversion_mode_prev {
            self.compartment.set_u32(conversion_mode).ok();
        }
    }

    pub fn keyboard_open_compartment_updated(&self, thread_mgr: ITfThreadMgr) {
        let conversion_mode = self.compartment.get_u32();
        if conversion_mode.is_err() {
            return;
        }
        let mut conversion_mode = conversion_mode.unwrap();
        let conversion_mode_prev = conversion_mode;

        conversion_mode = Self::updated_conversion_mode(
            conversion_mode,
            thread_mgr,
            self.tf_client_id,
            GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
            TF_CONVERSIONMODE_NATIVE,
        )
        .unwrap_or(conversion_mode);

        if conversion_mode != conversion_mode_prev {
            self.compartment.set_u32(conversion_mode).ok();
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn compartment_callback(
    wrapper: *const std::ffi::c_void,
    guid: &windows::core::GUID,
) -> HRESULT {
    let wrapper = wrapper as *const CompartmentUpdateListener;

    let thread_mgr: windows::core::Result<ITfThreadMgr> =
        CoCreateInstance(&CLSID_TF_ThreadMgr, None, CLSCTX_INPROC_SERVER);

    if thread_mgr.is_err() {
        return HRESULT::from(thread_mgr);
    }

    let thread_mgr = thread_mgr.unwrap();
    let wrapper = wrapper.as_ref().unwrap();

    if guid == &SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE
        || guid == &SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION
    {
        wrapper.private_compartments_updated(thread_mgr);
    } else if guid == &GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION
        || guid == &GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE
    {
        wrapper.conversion_mode_compartment_updated(thread_mgr);
    } else if guid == &GUID_COMPARTMENT_KEYBOARD_OPENCLOSE {
        wrapper.keyboard_open_compartment_updated(thread_mgr);
    }

    HRESULT::from(Ok(()))
}
