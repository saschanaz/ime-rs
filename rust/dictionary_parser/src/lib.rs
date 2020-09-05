use core::ffi::c_void;
use ruststringrange::RustStringRange;

#[no_mangle]
pub unsafe extern fn parse_line(line: *const c_void, key: *mut *mut c_void, value: *mut *mut c_void) -> bool {
    fn unwrap(text: &str) -> &str {
        let mut result = text;
        if text.starts_with("\"") && text.ends_with("\"") {
            result = &text[1..text.len()-1];
        }
        result.trim()
    }

    let line = Box::leak(RustStringRange::from_void(line as *mut _));
    let split: Vec<&str> = line.as_slice().split('=').collect();
    if split.len() < 2 {
        return false;
    }
    *key = Box::into_raw(Box::new(RustStringRange::from_str(unwrap(split[0])))) as *mut c_void;
    *value = Box::into_raw(Box::new(RustStringRange::from_str(unwrap(split[1])))) as *mut c_void;
    return true;
}

#[no_mangle]
pub unsafe extern fn get_equalsign(buffer: *const u16, buffer_len: usize) -> *mut u16 {
    let slice: &[u16] = std::slice::from_raw_parts(buffer, buffer_len);

    // ignore equalsign wrapped in doublequote
    let mut in_quote = false;
    let position = slice.iter().position(|&c| {
        if c == b'\"'.into() {
            in_quote = !in_quote;
        }
        if in_quote {
            false
        } else {
            c == b'='.into()
        }
    });

    match position {
        Some(p) => buffer.offset(p as isize) as *mut u16,
        None => 0 as *mut u16
    }
}

#[cfg(test)]
mod tests {
    mod line_parser {
        use super::super::*;

        #[test]
        fn parse() {
            let line_raw = Box::into_raw(Box::new(RustStringRange::from_str("\"abc\"=\"bcd\"")));
            let mut key_raw = std::ptr::null::<c_void>() as *mut c_void;
            let mut value_raw = key_raw;
            unsafe {
                let result = parse_line(line_raw as *mut c_void, &mut key_raw, &mut value_raw);
                assert_eq!(result, true);
                Box::from_raw(line_raw); // implicit destruction
                let key = Box::from_raw(key_raw as *mut RustStringRange);
                let value = Box::from_raw(value_raw as *mut RustStringRange);
                assert_eq!(key.as_slice(), "abc");
                assert_eq!(value.as_slice(), "bcd");
            }
        }
    }

    mod equalsign_getter {
        fn utf16(s: &str) -> Vec<u16> {
            s.encode_utf16().collect()
        }

        #[test]
        fn equalsign() {
            let encoded = utf16("abc=");
            let ptr = encoded.as_ptr();

            unsafe {
                let result = crate::get_equalsign(ptr, encoded.len());
                assert_eq!(result, ptr.offset(3) as *mut u16);
                let char_code = *result as u16;
                assert_eq!(char_code, b'='.into());
            }
        }

        #[test]
        fn equalsign_cut() {
            let encoded = utf16("abc=");
            let ptr = encoded.as_ptr();

            unsafe {
                let result = crate::get_equalsign(ptr, 2);
                assert_eq!(result, 0 as *mut u16);
            }
        }

        #[test]
        fn equalsign_wrapped() {
            let encoded = utf16("\"abc=\"=");
            let ptr = encoded.as_ptr();

            unsafe {
                let result = crate::get_equalsign(ptr, encoded.len());
                assert_eq!(result, ptr.offset(6) as *mut u16);
                let char_code = *result as u16;
                assert_eq!(char_code, b'='.into());
            }
        }

        #[test]
        fn equalsign_wrapped_nomatch() {
            let encoded = utf16("\"abc=\"");
            let ptr = encoded.as_ptr();

            unsafe {
                let result = crate::get_equalsign(ptr, encoded.len());
                assert_eq!(result, 0 as *mut u16);
            }
        }
    }
}
