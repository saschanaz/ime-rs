use windows::runtime::HRESULT;
use windows::Win32::Foundation::{HINSTANCE, S_OK};

mod registry;

mod com;

#[no_mangle]
pub extern "C" fn register_profile(dll_instance_handle: HINSTANCE) -> bool {
    registry::register_profile(dll_instance_handle).is_ok()
}

#[no_mangle]
pub extern "C" fn register_categories() -> bool {
    registry::register_categories().is_ok()
}

#[no_mangle]
pub extern "C" fn register_server(dll_instance_handle: HINSTANCE) -> bool {
    registry::register_server(dll_instance_handle).is_ok()
}

#[no_mangle]
#[allow(non_snake_case)]
#[doc(hidden)]
pub unsafe extern "system" fn DllUnregisterServer() -> HRESULT {
    registry::unregister_profile().ok();
    registry::unregister_categories().ok();
    registry::unregister_server().ok();
    S_OK
}
