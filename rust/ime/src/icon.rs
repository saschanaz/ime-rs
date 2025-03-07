use windows::core::PCWSTR;
use windows::Win32::Foundation::HINSTANCE;
use windows::Win32::UI::WindowsAndMessaging::{LoadImageW, HICON, IMAGE_FLAGS, IMAGE_ICON};

use crate::dll::DLL_INSTANCE;

pub fn get_icon(desired_size: i32, index: u32) -> windows::core::Result<HICON> {
    let dll_instance = unsafe { DLL_INSTANCE };
    if dll_instance.0.is_null() {
        return Ok(HICON(std::ptr::null_mut()));
    }

    let icon = unsafe {
        LoadImageW(
            Some(HINSTANCE(dll_instance.0)),
            // (Mocking MAKEINTRESOURCE)
            PCWSTR(index as usize as *const u16),
            IMAGE_ICON,
            desired_size,
            desired_size,
            IMAGE_FLAGS(0),
        )
    }?;
    Ok(HICON(icon.0))
}
