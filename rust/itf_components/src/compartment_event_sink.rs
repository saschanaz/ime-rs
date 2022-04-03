#![allow(clippy::missing_safety_doc)]
// https://github.com/microsoft/windows-rs/issues/1506
#![allow(clippy::not_unsafe_ptr_arg_deref)]

use std::ffi::c_void;

use windows::core::{implement, IUnknown, Interface, ToImpl, GUID, HRESULT};
use windows::Win32::UI::TextServices::{
    ITfCompartment, ITfCompartmentEventSink, ITfCompartmentEventSink_Impl, ITfCompartmentMgr,
    ITfSource,
};

pub type CESCALLBACK = unsafe extern "C" fn(pv: *const c_void, guid: *const GUID) -> HRESULT;

#[implement(ITfCompartmentEventSink)]
pub struct CompartmentEventSink {
    callback: CESCALLBACK,
    pv: *const c_void,
    compartment: Option<ITfCompartment>,
    cookie: u32,
}

impl CompartmentEventSink {
    pub fn new(callback: CESCALLBACK, pv: *const c_void) -> CompartmentEventSink {
        CompartmentEventSink {
            callback,
            pv,
            compartment: None,
            cookie: 0,
        }
    }

    pub fn advise(
        sink: ITfCompartmentEventSink,
        punk: IUnknown,
        guid: &GUID,
    ) -> windows::core::Result<()> {
        let manager: ITfCompartmentMgr = punk.cast()?;

        unsafe {
            let sink_impl = CompartmentEventSink::to_impl(&sink);
            sink_impl.compartment = Some(manager.GetCompartment(guid)?);
            let source: ITfSource = sink_impl.compartment.as_mut().unwrap().cast()?;
            sink_impl.cookie = source.AdviseSink(&ITfCompartmentEventSink::IID, &sink)?;
        }
        Ok(())
    }

    pub fn unadvise(sink: ITfCompartmentEventSink) -> windows::core::Result<()> {
        let sink_impl = unsafe { CompartmentEventSink::to_impl(&sink) };
        if let Some(ref mut compartment) = sink_impl.compartment {
            let source: ITfSource = compartment.cast()?;
            unsafe {
                source.UnadviseSink(sink_impl.cookie)?;
            }
        }
        sink_impl.compartment = None;
        sink_impl.cookie = 0;
        Ok(())
    }
}

impl ITfCompartmentEventSink_Impl for CompartmentEventSink {
    fn OnChange(&self, guid: *const GUID) -> windows::core::Result<()> {
        unsafe { (self.callback)(self.pv, guid).ok() }
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
