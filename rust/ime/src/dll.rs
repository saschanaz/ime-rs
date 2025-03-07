use windows::core::HRESULT;
use windows::Win32::Foundation::{E_FAIL, HMODULE, S_OK};

use crate::registry;

#[no_mangle]
pub static mut DLL_INSTANCE: HMODULE = HMODULE(std::ptr::null_mut());

#[no_mangle]
#[allow(non_snake_case)]
#[doc(hidden)]
unsafe extern "system" fn DllRegisterServer() -> HRESULT {
    unsafe fn register() -> windows::core::Result<()> {
        registry::register_server(DLL_INSTANCE)
            .map_err(|_| windows::core::Error::new(E_FAIL, "Failed to register server"))?;
        registry::register_profile(DLL_INSTANCE)?;
        registry::register_categories()?;
        Ok(())
    }

    // See https://github.com/rust-lang/rust-clippy/issues/13185
    #[allow(clippy::manual_inspect)]
    register()
        .map_err(|err| {
            DllUnregisterServer().ok().ok();
            err
        })
        .into()
}

#[no_mangle]
#[allow(non_snake_case)]
#[doc(hidden)]
unsafe extern "system" fn DllUnregisterServer() -> HRESULT {
    registry::unregister_profile().ok();
    registry::unregister_categories().ok();
    registry::unregister_server().ok();
    S_OK
}
