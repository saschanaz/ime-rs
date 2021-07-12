mod bindings;
use bindings::Windows::Win32::UI::WindowsAndMessaging::{VK_NUMPAD0, VK_NUMPAD9};

pub fn is_number_key(vkey: u32) -> bool {
    index_from_number_key(vkey) != -1
}

#[no_mangle]
pub extern "C" fn index_from_number_key(vkey: u32) -> i32 {
    let value: i32 = if (VK_NUMPAD0..=VK_NUMPAD9).contains(&vkey) {
        vkey as i32 - VK_NUMPAD0 as i32
    } else {
        vkey as i32 - '0' as i32
    };

    if value == 0 {
        9
    } else if value > 0 && value < 10 {
        value as i32 - 1
    } else {
        -1
    }
}

#[no_mangle]
pub extern "C" fn number_key_label_at(index: u32) -> u32 {
    assert!(index < 10);
    if index == 9 {
        0
    } else {
        index + 1
    }
}

#[cfg(test)]
mod tests {
    use super::VK_NUMPAD0;
    const VK_NUMPAD1: u32 = VK_NUMPAD0 + 1;
    const VK_NUMPAD2: u32 = VK_NUMPAD0 + 2;
    const VK_DIVIDE: u32 = VK_NUMPAD0 + 14;

    use crate::{index_from_number_key, is_number_key, number_key_label_at};

    #[test]
    fn check_number_key() {
        assert!(is_number_key(VK_NUMPAD1 as u32));
        assert!(!is_number_key(VK_DIVIDE as u32));
    }

    #[test]
    fn index() {
        assert!(index_from_number_key(VK_NUMPAD2 as u32) == 1);
        assert!(index_from_number_key('3' as u32) == 2);
    }

    #[test]
    fn label() {
        assert!(number_key_label_at(1) == 2);
    }
}
