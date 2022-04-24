use windows::core::PCWSTR;
use windows::Win32::UI::WindowsAndMessaging::{LoadImageW, HICON, IMAGE_FLAGS, IMAGE_ICON};

use crate::dll::DLL_INSTANCE;

pub fn get_icon(desired_size: i32, index: u32) -> windows::core::Result<HICON> {
    let dll_instance = unsafe { DLL_INSTANCE };
    if dll_instance.0 == 0 {
        return Ok(HICON(0));
    }

    let icon = unsafe {
        LoadImageW(
            dll_instance,
            // (Mocking MAKEINTRESOURCE)
            PCWSTR(core::mem::transmute::<usize, *const u16>(index as usize)),
            IMAGE_ICON,
            desired_size,
            desired_size,
            IMAGE_FLAGS(0),
        )
    }?;
    Ok(HICON(icon.0))
}
