use std::ffi::c_void;

use globals::{
    SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE, SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION,
};
use ime::resources::{
    IME_MODE_DESCRIPTION, IME_MODE_OFF_ICO_INDEX, IME_MODE_ON_ICO_INDEX,
    LANGBAR_IME_MODE_DESCRIPTION,
};
use itf_components::{
    compartment_event_sink::CompartmentEventSink, language_bar::LangBarItemButton,
};
use windows::Win32::UI::TextServices::{
    ITfCompartmentEventSink, ITfLangBarItemButton, ITfThreadMgr,
    GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
    GUID_LBI_INPUTMODE,
};

use super::compartment_update_listener::{compartment_callback, CompartmentUpdateListener};

pub struct LanguageBar {
    ime_mode_button: ITfLangBarItemButton,

    keyboard_open_event_sink: Option<ITfCompartmentEventSink>,
    conversion_event_sink: Option<ITfCompartmentEventSink>,
    double_single_byte_event_sink: Option<ITfCompartmentEventSink>,
    punctuation_event_sink: Option<ITfCompartmentEventSink>,
}

impl LanguageBar {
    fn init_button() -> LangBarItemButton {
        LangBarItemButton::new(
            GUID_LBI_INPUTMODE,
            LANGBAR_IME_MODE_DESCRIPTION,
            IME_MODE_DESCRIPTION,
            IME_MODE_ON_ICO_INDEX,
            IME_MODE_OFF_ICO_INDEX,
        )
    }

    pub fn button(&self) -> &ITfLangBarItemButton {
        &self.ime_mode_button
    }

    pub fn new() -> LanguageBar {
        let ime_mode_button = LanguageBar::init_button();
        LanguageBar {
            ime_mode_button: ime_mode_button.into(),
            keyboard_open_event_sink: None,
            conversion_event_sink: None,
            double_single_byte_event_sink: None,
            punctuation_event_sink: None,
        }
    }

    pub fn init(
        &mut self,
        thread_mgr: ITfThreadMgr,
        client_id: u32,
        compartment_wrapper: &CompartmentUpdateListener,
    ) -> windows::core::Result<()> {
        LangBarItemButton::init(
            self.ime_mode_button.clone(),
            thread_mgr.clone(),
            client_id,
            &GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
        )?;

        let pv: *const c_void = compartment_wrapper as *const _ as _;

        self.keyboard_open_event_sink =
            Some(CompartmentEventSink::new(compartment_callback, pv).into());
        self.conversion_event_sink =
            Some(CompartmentEventSink::new(compartment_callback, pv).into());
        self.double_single_byte_event_sink =
            Some(CompartmentEventSink::new(compartment_callback, pv).into());
        self.punctuation_event_sink =
            Some(CompartmentEventSink::new(compartment_callback, pv).into());

        CompartmentEventSink::advise(
            self.keyboard_open_event_sink.clone().unwrap(),
            thread_mgr.clone().into(),
            &GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
        )?;
        CompartmentEventSink::advise(
            self.conversion_event_sink.clone().unwrap(),
            thread_mgr.clone().into(),
            &GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION,
        )?;
        CompartmentEventSink::advise(
            self.double_single_byte_event_sink.clone().unwrap(),
            thread_mgr.clone().into(),
            &SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE,
        )?;
        CompartmentEventSink::advise(
            self.punctuation_event_sink.clone().unwrap(),
            thread_mgr.into(),
            &SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION,
        )?;

        Ok(())
    }
}

impl Drop for LanguageBar {
    fn drop(&mut self) {
        fn unadvise(sink: &mut Option<ITfCompartmentEventSink>) {
            if sink.is_some() {
                CompartmentEventSink::unadvise(sink.take().unwrap()).ok();
            }
        }

        LangBarItemButton::cleanup(self.button().clone());

        unadvise(&mut self.keyboard_open_event_sink);
        unadvise(&mut self.conversion_event_sink);
        unadvise(&mut self.double_single_byte_event_sink);
        unadvise(&mut self.punctuation_event_sink);
    }
}
