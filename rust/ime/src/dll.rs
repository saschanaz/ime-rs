use windows::runtime::HRESULT;
use windows::Win32::Foundation::{HINSTANCE, S_OK};

use crate::registry;

#[no_mangle]
pub static mut DLL_INSTANCE: HINSTANCE = HINSTANCE { 0: 0 };

#[no_mangle]
#[allow(non_snake_case)]
#[doc(hidden)]
pub unsafe extern "system" fn DllUnregisterServer() -> HRESULT {
    registry::unregister_profile().ok();
    registry::unregister_categories().ok();
    registry::unregister_server().ok();
    S_OK
}
