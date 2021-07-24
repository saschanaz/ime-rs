use crate::bindings::Windows::Win32::System::Com::{CoCreateInstance, CLSCTX_INPROC_SERVER};
use windows::{Guid, Interface, Result};

pub fn create_instance_inproc<T: Interface>(clsid: &Guid) -> Result<T> {
    unsafe { CoCreateInstance(clsid, None, CLSCTX_INPROC_SERVER) }
}
