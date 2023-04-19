use windows::Win32::Foundation::HWND;

use crate::base_window::BaseWindow;

pub trait Window {
    fn create(
        atom: u16,
        ex_style: u32,
        style: u32,
        parent_window: *mut dyn Window,
        window_width: u16,
        window_height: u16,
        parent_window_handle: HWND,
    ) where
        Self: Sized;

    fn get_window() -> HWND
    where
        Self: Sized;
}
