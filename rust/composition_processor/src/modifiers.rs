use crate::bindings::Windows::Win32::Foundation::{LPARAM, WPARAM};
use crate::bindings::Windows::Win32::UI::KeyboardAndMouseInput::GetKeyState;
use crate::bindings::Windows::Win32::UI::WindowsAndMessaging::{VK_CONTROL, VK_MENU, VK_SHIFT};

// msctf.h
static TF_MOD_ALT: u16 = 0x0001;
static TF_MOD_CONTROL: u16 = 0x0002;
static TF_MOD_SHIFT: u16 = 0x0004;
static TF_MOD_RALT: u16 = 0x0008;
static TF_MOD_RCONTROL: u16 = 0x0010;
static TF_MOD_RSHIFT: u16 = 0x0020;
static TF_MOD_LALT: u16 = 0x0040;
static TF_MOD_LCONTROL: u16 = 0x0080;
static TF_MOD_LSHIFT: u16 = 0x0100;

#[derive(Default)]
pub struct Modifiers {
    value: u16,
    is_shift_key_down_only: bool,
    is_control_key_down_only: bool,
    is_alt_key_down_only: bool,
}

impl Modifiers {
    pub unsafe fn from_void(modifiers: *mut std::ffi::c_void) -> Box<Modifiers> {
        Box::from_raw(modifiers as *mut Modifiers)
    }

    pub fn update(&mut self, w: WPARAM, l: LPARAM) {
        // high-order bit : key down
        // low-order bit  : toggled
        let ks_menu = unsafe { GetKeyState(VK_MENU as i32) } as u16;
        let ks_control = unsafe { GetKeyState(VK_CONTROL as i32) } as u16;
        let ks_shift = unsafe { GetKeyState(VK_SHIFT as i32) } as u16;

        match w.0 as u32 & 0xff {
            VK_MENU => {
                // is VK_MENU down?
                if ks_menu & 0x8000 != 0 {
                    // is extended key?
                    if l.0 & 0x01000000 != 0 {
                        self.value |= TF_MOD_RALT | TF_MOD_ALT;
                    } else {
                        self.value |= TF_MOD_LALT | TF_MOD_ALT;
                    }
                }

                // is previous key state up?
                if (l.0 & 0x40000000) == 0 {
                    // is VK_CONTROL and VK_SHIFT up?
                    if (ks_control & 0x8000 == 0) && (ks_shift & 0x8000 == 0) {
                        self.is_alt_key_down_only = true;
                    } else {
                        self.is_shift_key_down_only = false;
                        self.is_control_key_down_only = false;
                        self.is_alt_key_down_only = false;
                    }
                }
            }
            VK_CONTROL => {
                // is VK_CONTROL down?
                if ks_control & 0x8000 != 0 {
                    // is extended key?
                    if l.0 & 0x01000000 != 0 {
                        self.value |= TF_MOD_RCONTROL | TF_MOD_CONTROL;
                    } else {
                        self.value |= TF_MOD_LCONTROL | TF_MOD_CONTROL;
                    }
                }

                // is previous key state up?
                if (l.0 & 0x40000000) == 0 {
                    // is VK_SHIFT and VK_MENU up?
                    if (ks_shift & 0x8000 == 0) && (ks_menu & 0x8000 == 0) {
                        self.is_control_key_down_only = true;
                    } else {
                        self.is_shift_key_down_only = false;
                        self.is_control_key_down_only = false;
                        self.is_alt_key_down_only = false;
                    }
                }
            }
            VK_SHIFT => {
                // is VK_SHIFT down?
                if ks_shift & 0x8000 != 0 {
                    // is scan code 0x36(right shift)?
                    if (l.0 >> 16) & 0x00ff == 0x36 {
                        self.value |= TF_MOD_RSHIFT | TF_MOD_SHIFT;
                    } else {
                        self.value |= TF_MOD_LSHIFT | TF_MOD_SHIFT;
                    }
                }

                // is previous key state up?
                if (l.0 & 0x40000000) == 0 {
                    // is VK_SHIFT and VK_MENU up?
                    if (ks_menu & 0x8000 == 0) && (ks_control & 0x8000 == 0) {
                        self.is_shift_key_down_only = true;
                    } else {
                        self.is_shift_key_down_only = false;
                        self.is_control_key_down_only = false;
                        self.is_alt_key_down_only = false;
                    }
                }
            }
            _ => {
                self.is_shift_key_down_only = false;
                self.is_control_key_down_only = false;
                self.is_alt_key_down_only = false;
            }
        }

        if ks_menu & 0x8000 == 0 {
            self.value &= !(TF_MOD_RALT | TF_MOD_LALT | TF_MOD_ALT);
        }
        if ks_control & 0x8000 == 0 {
            self.value &= !(TF_MOD_RCONTROL | TF_MOD_LCONTROL | TF_MOD_CONTROL);
        }
        if ks_shift & 0x8000 == 0 {
            self.value &= !(TF_MOD_RSHIFT | TF_MOD_LSHIFT | TF_MOD_SHIFT);
        }
    }

    pub fn get(&self) -> u16 {
        self.value
    }

    pub fn is_shift_key_down_only(&self) -> bool {
        self.is_shift_key_down_only
    }

    pub fn is_control_key_down_only(&self) -> bool {
        self.is_control_key_down_only
    }

    pub fn is_alt_key_down_only(&self) -> bool {
        self.is_alt_key_down_only
    }
}
