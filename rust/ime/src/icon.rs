use windows::Win32::{
    Foundation::PWSTR,
    UI::WindowsAndMessaging::{LoadImageW, HICON, IMAGE_FLAGS, IMAGE_ICON},
};

use crate::dll::DLL_INSTANCE;

#[no_mangle]
pub unsafe extern "C" fn get_icon(desired_size: i32, index: u16) -> HICON {
    let dll_instance = DLL_INSTANCE;
    if dll_instance.0 == 0 {
        return HICON(0);
    }

    let icon = LoadImageW(
        dll_instance,
        // (Mocking MAKEINTRESOURCE)
        PWSTR(core::mem::transmute::<usize, *const u16>(index as usize)),
        IMAGE_ICON,
        desired_size,
        desired_size,
        IMAGE_FLAGS(0),
    );
    HICON(icon.0)
}
