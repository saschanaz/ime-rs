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

    *key = 0 as *mut c_void;
    *value = 0 as *mut c_void;

    let line = Box::leak(RustStringRange::from_void(line as *mut _));
    let equalsign = get_equalsign(line.as_slice());
    if equalsign.is_none() {
        return false;
    }

    let key_slice = &line.as_slice()[0..equalsign.unwrap()];
    let value_slice = &line.as_slice()[equalsign.unwrap() + 1..];
    *key = Box::into_raw(Box::new(RustStringRange::from_str(unwrap(key_slice)))) as *mut c_void;
    *value = Box::into_raw(Box::new(RustStringRange::from_str(unwrap(value_slice)))) as *mut c_void;
    return true;
}

pub fn get_equalsign(s: &str) -> Option<usize> {
    // ignore equalsign wrapped in doublequote
    let mut in_quote = false;
    let position = s.bytes().position(|c| {
        if c == b'\"' {
            in_quote = !in_quote;
        }
        if in_quote {
            false
        } else {
            c == b'='
        }
    });

    position
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

        #[test]
        fn parse_equalsign_wrapped() {
            let line_raw = Box::into_raw(Box::new(RustStringRange::from_str("\"a=bc\"=\"bc=d\"")));
            let mut key_raw = std::ptr::null::<c_void>() as *mut c_void;
            let mut value_raw = key_raw;
            unsafe {
                let result = parse_line(line_raw as *mut c_void, &mut key_raw, &mut value_raw);
                assert_eq!(result, true);
                Box::from_raw(line_raw); // implicit destruction
                let key = Box::from_raw(key_raw as *mut RustStringRange);
                let value = Box::from_raw(value_raw as *mut RustStringRange);
                assert_eq!(key.as_slice(), "a=bc");
                assert_eq!(value.as_slice(), "bc=d");
            }
        }
    }

    mod equalsign_getter {
        #[test]
        fn equalsign() {
            let s = "abc=";

            let result = crate::get_equalsign(s).unwrap();
            assert_eq!(result, 3);
            let char_code = s.bytes().nth(result).unwrap();
            assert_eq!(char_code, b'=');
        }

        #[test]
        fn equalsign_wrapped() {
            let s = "\"abc=\"=";

            let result = crate::get_equalsign(s).unwrap();
            assert_eq!(result, 6);
            let char_code = s.bytes().nth(result).unwrap();
            assert_eq!(char_code, b'=');
        }

        #[test]
        fn equalsign_wrapped_nomatch() {
            let s = "\"abc=\"";

            let result = crate::get_equalsign(s);
            assert!(result.is_none());
        }
    }
}
