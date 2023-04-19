use windows::Win32::Foundation::{HWND, RECT};

use crate::window_trait::Window;

pub struct BaseWindow {
    handle: HWND,

    // TODO: use Box<>
    parent_window: *mut dyn Window,
    ui_window: *mut dyn Window,

    timer_ui_obj: *mut dyn Window,
    ui_obj_capture: *mut dyn Window,

    enable_virtual_window: bool,
    visible_virtual_window: bool,
    virtual_window_rect: RECT,
}
