use windows::core::{Interface, Result, GUID};
use windows::Win32::System::Com::{CoCreateInstance, CLSCTX_INPROC_SERVER};

pub fn create_instance_inproc<T: Interface + windows::core::ComInterface>(
    clsid: &GUID,
) -> Result<T> {
    unsafe { CoCreateInstance(clsid, None, CLSCTX_INPROC_SERVER) }
}
