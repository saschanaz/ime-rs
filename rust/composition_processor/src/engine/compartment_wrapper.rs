use globals::{
    SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE, SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION,
};
use itf_components::compartment::Compartment;
use windows::Win32::UI::TextServices::{
    ITfThreadMgr, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION,
    GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, TF_CONVERSIONMODE_FULLSHAPE, TF_CONVERSIONMODE_NATIVE,
    TF_CONVERSIONMODE_SYMBOL,
};

pub struct CompartmentWrapper {
    compartment: Compartment,
    tf_client_id: u32,
}

impl CompartmentWrapper {
    pub fn new(thread_mgr: ITfThreadMgr, tf_client_id: u32) -> CompartmentWrapper {
        CompartmentWrapper {
            compartment: Compartment::new(
                &Some(thread_mgr.into()),
                tf_client_id,
                GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION,
            ),
            tf_client_id,
        }
    }

    fn toggle_compartment(
        conversion_mode: u32,
        thread_mgr: ITfThreadMgr,
        tf_client_id: u32,
        guid: windows::core::GUID,
        tf_flag: u32,
    ) -> windows::core::Result<()> {
        let compartment = Compartment::new(&Some(thread_mgr.into()), tf_client_id, guid);
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
}
