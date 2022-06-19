use windows::Win32::UI::Input::KeyboardAndMouse::{VK_NUMPAD0, VK_NUMPAD9};

#[no_mangle]
pub extern "C" fn index_from_number_key(vkey: u16) -> i16 {
    let value: i16 = if (VK_NUMPAD0.0..=VK_NUMPAD9.0).contains(&vkey) {
        vkey as i16 - VK_NUMPAD0.0 as i16
    } else {
        vkey as i16 - '0' as i16
    };

    if value == 0 {
        9
    } else if value > 0 && value < 10 {
        value as i16 - 1
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
    use windows::Win32::UI::Input::KeyboardAndMouse::VK_NUMPAD2;

    use crate::{index_from_number_key, number_key_label_at};

    #[test]
    fn index() {
        assert!(index_from_number_key(VK_NUMPAD2.0 as _) == 1);
        assert!(index_from_number_key('3' as _) == 2);
    }

    #[test]
    fn label() {
        assert!(number_key_label_at(1) == 2);
    }
}
