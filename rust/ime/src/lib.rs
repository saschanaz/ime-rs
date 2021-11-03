use windows::Win32::Foundation::HINSTANCE;

mod registry;
pub use registry::unregister_server;

mod com;

#[no_mangle]
pub extern "C" fn register_profile(dll_instance_handle: HINSTANCE) -> bool {
    registry::register_profile(dll_instance_handle).is_ok()
}

#[no_mangle]
pub extern "C" fn unregister_profile() -> bool {
    registry::unregister_profile().is_ok()
}

#[no_mangle]
pub extern "C" fn register_categories() -> bool {
    registry::register_categories().is_ok()
}

#[no_mangle]
pub extern "C" fn unregister_categories() -> bool {
    registry::unregister_categories().is_ok()
}

#[no_mangle]
pub extern "C" fn register_server(dll_instance_handle: HINSTANCE) -> bool {
    registry::register_server(dll_instance_handle).is_ok()
}
