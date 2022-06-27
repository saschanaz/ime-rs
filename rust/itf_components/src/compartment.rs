#![allow(clippy::missing_safety_doc)]

use std::ffi::c_void;

use windows::core::{Interface, GUID, HRESULT};
use windows::Win32::System::Com::VARIANT;
use windows::Win32::System::Ole::VT_I4;
use windows::Win32::UI::TextServices::{
    ITfCompartment, ITfCompartmentMgr, ITfThreadMgr, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
};

pub struct Compartment {
    thread_mgr: ITfThreadMgr,
    tf_client_id: u32,
    guid: GUID,
}

impl Compartment {
    pub fn new(thread_mgr: ITfThreadMgr, tf_client_id: u32, guid: GUID) -> Compartment {
        Compartment {
            thread_mgr,
            tf_client_id,
            guid,
        }
    }

    pub fn read_bool(thread_mgr: ITfThreadMgr, tf_client_id: u32, guid: GUID) -> bool {
        Compartment::new(thread_mgr, tf_client_id, guid)
            .get_bool()
            .unwrap_or(false)
    }

    pub unsafe fn from_void(engine: *mut c_void) -> Box<Compartment> {
        Box::from_raw(engine as *mut Compartment)
    }

    fn get_compartment(&self) -> windows::core::Result<ITfCompartment> {
        let manager: ITfCompartmentMgr = self.thread_mgr.cast()?;
        unsafe { manager.GetCompartment(&self.guid) }
    }

    pub fn get_bool(&self) -> windows::core::Result<bool> {
        // VT_BOOL also exists and maybe preferrable for better strictness
        // The Microsoft demo was using VT_I4 here, so this just follows that.
        Ok(self.get_u32()? != 0)
    }

    pub fn set_bool(&self, flag: bool) -> windows::core::Result<()> {
        self.set_u32(flag.into())
    }

    pub fn get_u32(&self) -> windows::core::Result<u32> {
        unsafe {
            let variant = self.get_compartment()?.GetValue()?;
            if variant.Anonymous.Anonymous.vt != VT_I4.0 as u16 {
                return Ok(0); // Even VT_EMPTY, GetValue() can succeed
            }
            Ok(variant.Anonymous.Anonymous.Anonymous.lVal as u32)
        }
    }

    pub fn set_u32(&self, data: u32) -> windows::core::Result<()> {
        let mut variant = VARIANT::default();
        unsafe {
            (*variant.Anonymous.Anonymous).vt = VT_I4.0 as u16;
            (*variant.Anonymous.Anonymous).Anonymous.lVal = data as i32;
            self.get_compartment()?
                .SetValue(self.tf_client_id, &variant)
        }
    }

    pub fn clear(&self) -> windows::core::Result<()> {
        if self.guid == GUID_COMPARTMENT_KEYBOARD_OPENCLOSE {
            return Ok(());
        }
        let manager: ITfCompartmentMgr = self.thread_mgr.cast()?;
        unsafe { manager.ClearCompartment(self.tf_client_id, &self.guid) }
    }

    pub fn guid(&self) -> GUID {
        self.guid
    }
}

#[no_mangle]
pub unsafe extern "C" fn compartment_new(
    thread_mgr: ITfThreadMgr,
    tf_client_id: u32,
    guid: *const GUID,
) -> *mut c_void {
    Box::into_raw(Box::new(Compartment::new(thread_mgr, tf_client_id, *guid))) as *mut c_void
}

#[no_mangle]
pub unsafe extern "C" fn compartment_free(compartment: *mut c_void) {
    Compartment::from_void(compartment); // implicit cleanup
}

#[no_mangle]
pub unsafe extern "C" fn compartment_get_bool(
    compartment: *mut c_void,
    data: *mut bool,
) -> HRESULT {
    let compartment = Box::leak(Compartment::from_void(compartment));
    let result = compartment.get_bool();
    if let Ok(b) = result {
        *data = b
    }
    HRESULT::from(result)
}

#[no_mangle]
pub unsafe extern "C" fn compartment_set_bool(compartment: *mut c_void, flag: bool) -> HRESULT {
    let compartment = Box::leak(Compartment::from_void(compartment));
    HRESULT::from(compartment.set_bool(flag))
}

#[no_mangle]
pub unsafe extern "C" fn compartment_get_u32(compartment: *mut c_void, data: *mut u32) -> HRESULT {
    let compartment = Box::leak(Compartment::from_void(compartment));
    let result = compartment.get_u32();
    if let Ok(u) = result {
        *data = u
    }
    HRESULT::from(result)
}

#[no_mangle]
pub unsafe extern "C" fn compartment_set_u32(compartment: *mut c_void, data: u32) -> HRESULT {
    let compartment = Box::leak(Compartment::from_void(compartment));
    HRESULT::from(compartment.set_u32(data))
}

#[no_mangle]
pub unsafe extern "C" fn compartment_clear(compartment: *mut c_void) -> HRESULT {
    let compartment = Box::leak(Compartment::from_void(compartment));
    HRESULT::from(compartment.clear())
}

#[no_mangle]
pub unsafe extern "C" fn compartment_guid(compartment: *mut c_void, guid: *mut GUID) {
    let compartment = Box::leak(Compartment::from_void(compartment));
    *guid = compartment.guid();
}
