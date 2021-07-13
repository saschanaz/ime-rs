mod bindings;
use crate::bindings::Windows::Win32::Foundation::HINSTANCE;

mod registry;
pub use registry::unregister_server;

#[no_mangle]
pub extern "C" fn register_server(dll_instance_handle: HINSTANCE) -> bool {
    registry::register_server(dll_instance_handle).is_ok()
}
