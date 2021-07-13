#![allow(clippy::missing_safety_doc)]

mod registry;
use std::ffi::c_void;

pub use registry::unregister_server;
use ruststringrange::RustStringRange;

#[no_mangle]
pub unsafe extern "C" fn register_server(module_file_name: *mut c_void) -> bool {
    let module_file_name = Box::leak(RustStringRange::from_void(module_file_name));
    registry::register_server(module_file_name.as_slice()).is_ok()
}
