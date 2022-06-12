#![allow(clippy::missing_safety_doc)]
// https://github.com/microsoft/windows-rs/issues/1506
#![allow(clippy::not_unsafe_ptr_arg_deref)]

use std::cell::{Cell, RefCell};
use std::ffi::c_void;

use windows::core::{implement, AsImpl, IUnknown, Interface, GUID, HRESULT};
use windows::Win32::UI::TextServices::{
    ITfCompartment, ITfCompartmentEventSink, ITfCompartmentEventSink_Impl, ITfCompartmentMgr,
    ITfSource,
};

pub type CESCALLBACK = unsafe extern "C" fn(pv: *const c_void, guid: &GUID) -> HRESULT;

#[implement(ITfCompartmentEventSink)]
pub struct CompartmentEventSink {
    callback: CESCALLBACK,
    pv: *const c_void,
    compartment: RefCell<Option<ITfCompartment>>,
    cookie: Cell<u32>,
}

impl CompartmentEventSink {
    pub fn new(callback: CESCALLBACK, pv: *const c_void) -> CompartmentEventSink {
        CompartmentEventSink {
            callback,
            pv,
            compartment: RefCell::new(None),
            cookie: Cell::new(0),
        }
    }

    pub fn advise(
        sink: ITfCompartmentEventSink,
        punk: IUnknown,
        guid: &GUID,
    ) -> windows::core::Result<()> {
        let manager: ITfCompartmentMgr = punk.cast()?;

        unsafe {
            let sink_impl: &CompartmentEventSink = sink.as_impl();
            let compartment = manager.GetCompartment(guid)?;
            let source: ITfSource = compartment.cast()?;

            sink_impl.compartment.replace(Some(compartment));
            sink_impl
                .cookie
                .replace(source.AdviseSink(&ITfCompartmentEventSink::IID, &sink)?);
        }
        Ok(())
    }

    pub fn unadvise(sink: ITfCompartmentEventSink) -> windows::core::Result<()> {
        let sink_impl: &CompartmentEventSink = sink.as_impl();
        if let Some(compartment) = sink_impl.compartment.borrow_mut().as_mut() {
            let source: ITfSource = compartment.cast()?;
            unsafe {
                source.UnadviseSink(sink_impl.cookie.get())?;
            }
        }
        sink_impl.compartment.replace(None);
        sink_impl.cookie.replace(0);
        Ok(())
    }
}

impl ITfCompartmentEventSink_Impl for CompartmentEventSink {
    fn OnChange(&self, guid: *const GUID) -> windows::core::Result<()> {
        unsafe { (self.callback)(self.pv, &*guid).ok() }
    }
}

#[no_mangle]
pub unsafe extern "C" fn compartmenteventsink_new(
    callback: CESCALLBACK,
    pv: *const c_void,
) -> *mut c_void {
    let sink = CompartmentEventSink::new(callback, pv);
    let sink: ITfCompartmentEventSink = sink.into();
    core::mem::transmute(sink)
}

#[no_mangle]
pub unsafe extern "C" fn compartmenteventsink_advise(
    sink: ITfCompartmentEventSink,
    punk: IUnknown,
    guid: *const GUID,
) -> HRESULT {
    HRESULT::from(CompartmentEventSink::advise(sink, punk, &*guid))
}

#[no_mangle]
pub unsafe extern "C" fn compartmenteventsink_unadvise(sink: ITfCompartmentEventSink) -> HRESULT {
    HRESULT::from(CompartmentEventSink::unadvise(sink))
}
