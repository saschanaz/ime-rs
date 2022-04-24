#![allow(clippy::missing_safety_doc)]
// https://github.com/microsoft/windows-rs/issues/1506
#![allow(clippy::not_unsafe_ptr_arg_deref)]

use std::ffi::c_void;

use globals::SAMPLEIME_CLSID;
use ruststringrange::RustStringRange;
use windows::core::{implement, IUnknown, Interface, ToImpl, GUID, HRESULT};
use windows::Win32::Foundation::{BOOL, BSTR, E_FAIL, E_INVALIDARG, POINT, RECT};
use windows::Win32::System::Com::{CoCreateInstance, CLSCTX_INPROC_SERVER};
use windows::Win32::System::Ole::{
    CONNECT_E_ADVISELIMIT, CONNECT_E_CANNOTCONNECT, CONNECT_E_NOCONNECTION,
};
use windows::Win32::UI::TextServices::*;
use windows::Win32::UI::WindowsAndMessaging::HICON;

use crate::compartment::Compartment;
use crate::compartment_event_sink::CompartmentEventSink;

#[implement(ITfLangBarItemButton, ITfSource)]
pub struct LangBarItemButton {
    info: TF_LANGBARITEMINFO,
    lang_bar_item_sink: std::cell::RefCell<Option<ITfLangBarItemSink>>,
    on_icon_index: u32,
    off_icon_index: u32,
    tooltip: String,

    added_to_lang_bar: bool,
    status: u32,

    compartment: Option<Compartment>,
    compartment_event_sink: Option<ITfCompartmentEventSink>,

    // The cookie for the sink to CLangBarItemButton.
    // Always 0 per the current implementation.
    cookie: u32,
}

impl LangBarItemButton {
    pub fn new(
        item_guid: GUID,
        description: &str,
        tooltip: &str,
        on_icon_index: u32,
        off_icon_index: u32,
    ) -> LangBarItemButton {
        let mut desc: Vec<u16> = description.encode_utf16().collect();
        desc.resize(32, 0);
        LangBarItemButton {
            // Initialize TF_LANGBARITEMINFO structure.
            info: TF_LANGBARITEMINFO {
                clsidService: SAMPLEIME_CLSID,
                guidItem: item_guid,
                dwStyle: TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY,
                ulSort: 0,
                szDescription: desc.try_into().unwrap(),
            },

            // Initialize the sink pointer to None.
            // Note: windows-rs 0.33+ uses &self for methods, and thus any mutation requires RefCell.
            // See also https://github.com/microsoft/windows-rs/pull/1511
            lang_bar_item_sink: std::cell::RefCell::new(None),

            // Initialize ICON index.
            on_icon_index,
            off_icon_index,

            // Initialize compartment.
            compartment: None,
            compartment_event_sink: None,

            added_to_lang_bar: false,
            status: 0,

            // Initialize Tooltip
            tooltip: tooltip.to_owned(),
            cookie: 0,
        }
    }

    fn add_item(
        button: ITfLangBarItemButton,
        thread_mgr: ITfThreadMgr,
    ) -> windows::core::Result<()> {
        unsafe {
            let button_impl = LangBarItemButton::to_impl(&button);
            if button_impl.added_to_lang_bar {
                return Ok(());
            }

            let lang_bar_item_mgr: ITfLangBarItemMgr = thread_mgr.cast()?;
            lang_bar_item_mgr.AddItem(button.clone())?;
            button_impl.added_to_lang_bar = true;
        }
        Ok(())
    }

    fn remove_item(
        button: ITfLangBarItemButton,
        thread_mgr: ITfThreadMgr,
    ) -> windows::core::Result<()> {
        unsafe {
            let button_impl = LangBarItemButton::to_impl(&button);
            if !button_impl.added_to_lang_bar {
                return Ok(());
            }

            let lang_bar_item_mgr: ITfLangBarItemMgr = thread_mgr.cast()?;
            lang_bar_item_mgr.RemoveItem(button.clone())?;
            button_impl.added_to_lang_bar = false;
        }
        Ok(())
    }

    fn register_compartment(
        button: ITfLangBarItemButton,
        thread_mgr: ITfThreadMgr,
        tf_client_id: u32,
        compartment_guid: &GUID,
    ) -> windows::core::Result<()> {
        unsafe {
            let button_impl = LangBarItemButton::to_impl(&button);
            button_impl.compartment = Some(Compartment::new(
                &Some(thread_mgr.cast()?),
                tf_client_id,
                *compartment_guid,
            ));

            // Advise ITfCompartmentEventSink
            let sink: ITfCompartmentEventSink = CompartmentEventSink::new(
                Self::compartment_callback,
                button_impl as *const _ as *const c_void,
            )
            .into();
            CompartmentEventSink::advise(sink.clone(), thread_mgr.cast()?, compartment_guid)?;
            button_impl.compartment_event_sink = Some(sink);
        }
        Ok(())
    }

    fn unregister_compartment(&self) -> windows::core::Result<()> {
        // Unadvise ITfCompartmentEventSink
        if let Some(compartment_event_sink) = self.compartment_event_sink.as_ref() {
            CompartmentEventSink::unadvise(compartment_event_sink.clone())?;
        }

        // clear ITfCompartment
        if let Some(compartment) = self.compartment.as_ref() {
            compartment.clear()?;
        }

        Ok(())
    }

    pub fn init(
        button: ITfLangBarItemButton,
        thread_mgr: ITfThreadMgr,
        tf_client_id: u32,
        compartment_guid: &GUID,
    ) -> windows::core::Result<()> {
        Self::add_item(button.clone(), thread_mgr.clone())?;
        Self::register_compartment(button, thread_mgr, tf_client_id, compartment_guid)?;
        Ok(())
    }

    pub fn cleanup(button: ITfLangBarItemButton) {
        let thread_mgr: windows::core::Result<ITfThreadMgr> =
            unsafe { CoCreateInstance(&ITfThreadMgr::IID, None, CLSCTX_INPROC_SERVER) };
        if let Ok(ref thread_mgr) = thread_mgr {
            Self::remove_item(button, thread_mgr.clone()).ok();
        }
    }

    pub fn set_status(&mut self, status: u32, set: bool) -> windows::core::Result<()> {
        let mut is_change = false;
        if set {
            if self.status & status == 0 {
                self.status |= status;
                is_change = true;
            }
        } else if self.status & status != 0 {
            self.status &= !status;
            is_change = true;
        }

        if is_change && self.lang_bar_item_sink.borrow().is_some() {
            unsafe {
                self.lang_bar_item_sink
                    .borrow()
                    .as_ref()
                    .unwrap()
                    .OnUpdate(TF_LBI_STATUS | TF_LBI_ICON)?;
            }
        }

        Ok(())
    }

    pub unsafe extern "C" fn compartment_callback(
        pv: *const std::ffi::c_void,
        compartment_guid: *const windows::core::GUID,
    ) -> HRESULT {
        let button_impl = pv as *mut LangBarItemButton;
        let button_impl = button_impl.as_ref().unwrap();

        if *compartment_guid == button_impl.compartment.as_ref().unwrap().guid() {
            if let Some(lang_bar_item_sink) = button_impl.lang_bar_item_sink.borrow().as_ref() {
                return HRESULT::from(lang_bar_item_sink.OnUpdate(TF_LBI_STATUS | TF_LBI_ICON));
            }
        }

        HRESULT::from(Ok(()))
    }
}

impl Drop for LangBarItemButton {
    fn drop(&mut self) {
        self.unregister_compartment().ok();
    }
}

impl ITfLangBarItem_Impl for LangBarItemButton {
    fn GetInfo(&self) -> windows::core::Result<TF_LANGBARITEMINFO> {
        Ok(self.info)
    }
    fn GetStatus(&self) -> windows::core::Result<u32> {
        Ok(self.status)
    }
    fn Show(&self, _fshow: BOOL) -> windows::core::Result<()> {
        unsafe {
            self.lang_bar_item_sink
                .borrow()
                .as_ref()
                .unwrap()
                .OnUpdate(TF_LBI_STATUS)
        }
    }
    fn GetTooltipString(&self) -> windows::core::Result<BSTR> {
        Ok(BSTR::from(&self.tooltip))
    }
}

impl ITfLangBarItemButton_Impl for LangBarItemButton {
    fn OnClick(
        &self,
        _click: TfLBIClick,
        _pt: &POINT,
        _prcarea: *const RECT,
    ) -> windows::core::Result<()> {
        let compartment = self.compartment.as_ref().unwrap();
        let is_on = compartment.get_bool()?;
        compartment.set_bool(!is_on)?;
        Ok(())
    }

    fn InitMenu(&self, _pmenu: &Option<ITfMenu>) -> windows::core::Result<()> {
        Ok(())
    }

    fn OnMenuSelect(&self, _wid: u32) -> windows::core::Result<()> {
        Ok(())
    }
    fn GetIcon(&self) -> windows::core::Result<HICON> {
        if self.compartment.is_none() {
            return Err(E_FAIL.ok().unwrap_err());
        }
        let is_on = self.compartment.as_ref().unwrap().get_bool()?;

        let status = self.GetStatus()?;

        // Ideally GetSystemMetrics() should be used, but here we just keep the original demo behavior.
        // https://docs.microsoft.com/en-us/windows/win32/api/ctfutb/nf-ctfutb-itflangbaritembutton-geticon
        let desired_size = 16;

        let index = if is_on && status & TF_LBI_STATUS_DISABLED == 0 {
            self.on_icon_index
        } else {
            self.off_icon_index
        };

        ime::icon::get_icon(desired_size, index)
    }

    fn GetText(&self) -> windows::core::Result<BSTR> {
        Ok(BSTR::from(&self.tooltip))
    }
}

impl ITfSource_Impl for LangBarItemButton {
    fn AdviseSink(&self, riid: *const GUID, punk: &Option<IUnknown>) -> windows::core::Result<u32> {
        // We allow only ITfLangBarItemSink interface.
        if unsafe { &*riid } != &ITfLangBarItemSink::IID {
            return Err(CONNECT_E_CANNOTCONNECT.ok().unwrap_err());
        }

        // We support only one sink once.
        if self.lang_bar_item_sink.borrow().is_some() {
            return Err(CONNECT_E_ADVISELIMIT.ok().unwrap_err());
        }

        // Query the ITfLangBarItemSink interface and store it into _pLangBarItemSink.
        if punk.is_none() {
            return Err(E_INVALIDARG.ok().unwrap_err());
        }
        self.lang_bar_item_sink
            .replace(Some(punk.as_ref().unwrap().cast()?));

        // return our cookie.
        Ok(self.cookie)
    }

    fn UnadviseSink(&self, dwcookie: u32) -> windows::core::Result<()> {
        // Check the given cookie.
        if dwcookie != self.cookie {
            return Err(CONNECT_E_NOCONNECTION.ok().unwrap_err());
        }

        // If there is no connected sink, we just fail.
        if self.lang_bar_item_sink.borrow().is_none() {
            return Err(CONNECT_E_NOCONNECTION.ok().unwrap_err());
        }

        self.lang_bar_item_sink.replace(None);

        Ok(())
    }
}

#[no_mangle]
pub unsafe extern "C" fn langbaritembutton_new(
    item_guid: *const GUID,
    description: *mut c_void,
    tooltip: *mut c_void,
    on_icon_index: u32,
    off_icon_index: u32,
) -> *mut c_void {
    let description = Box::leak(RustStringRange::from_void(description));
    let tooltip = Box::leak(RustStringRange::from_void(tooltip));
    let button = LangBarItemButton::new(
        *item_guid,
        description.as_slice(),
        tooltip.as_slice(),
        on_icon_index,
        off_icon_index,
    );
    let button: ITfLangBarItemButton = button.into();
    core::mem::transmute(button)
}

#[no_mangle]
pub unsafe extern "C" fn langbaritembutton_init(
    button: ITfLangBarItemButton,
    thread_mgr: ITfThreadMgr,
    tf_client_id: u32,
    compartment_guid: *const GUID,
) -> HRESULT {
    HRESULT::from(LangBarItemButton::init(
        button,
        thread_mgr,
        tf_client_id,
        &*compartment_guid,
    ))
}

#[no_mangle]
pub unsafe extern "C" fn langbaritembutton_cleanup(button: ITfLangBarItemButton) {
    LangBarItemButton::cleanup(button);
}

#[no_mangle]
pub unsafe extern "C" fn langbaritembutton_set_status(
    button: ITfLangBarItemButton,
    status: u32,
    set: bool,
) -> HRESULT {
    let button_impl = LangBarItemButton::to_impl(&button);
    HRESULT::from(button_impl.set_status(status, set))
}
