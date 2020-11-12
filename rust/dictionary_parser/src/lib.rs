use core::ffi::c_void;
use ruststringrange::RustStringRange;

fn parse_line_internal(line: &str) -> Option<(&str, &str)> {
    fn unwrap(text: &str) -> &str {
        let mut result = text;
        if text.starts_with("\"") && text.ends_with("\"") {
            result = &text[1..text.len()-1];
        }
        result.trim()
    }


    let equalsign = get_equalsign(line);
    if equalsign.is_none() {
        return None;
    }

    let key_slice = &line[0..equalsign.unwrap()];
    let value_slice = &line[equalsign.unwrap() + 1..];
    Some((unwrap(key_slice), unwrap(value_slice)))
}

#[no_mangle]
pub unsafe extern fn parse_line(line: *const c_void, key: *mut *mut c_void, value: *mut *mut c_void) -> bool {
    *key = 0 as *mut c_void;
    *value = 0 as *mut c_void;

    let line = Box::leak(RustStringRange::from_void(line as *mut _));
    let result = parse_line_internal(line.as_slice());
    if result.is_none() {
        return false;
    }

    let parsed = result.unwrap();
    *key = Box::into_raw(Box::new(RustStringRange::from_str(parsed.0))) as *mut c_void;
    *value = Box::into_raw(Box::new(RustStringRange::from_str(parsed.1))) as *mut c_void;
    return true;
}

fn find_all_internal<'a>(content: &'a str, search_key: &str, is_text_search: bool, is_wildcard_search: bool) -> Vec<(&'a str, &'a str)> {
    use compare_with_wildcard::compare_with_wildcard;

    let mut vec: Vec<(&'a str, &'a str)> = Vec::new();
    for line in content.lines() {
        let (key, value) = parse_line_internal(line).unwrap();
        let target = if is_text_search { value } else { key };
        let matches = if is_wildcard_search { compare_with_wildcard(search_key, target) } else { search_key.eq_ignore_ascii_case(target) };
        if matches {
            vec.push((key, value));
        }
    }
    vec
}

#[no_mangle]
pub unsafe extern fn find_all(content: *const c_void, search_key: *const c_void, is_text_search: bool, is_wildcard_search: bool, keys_buffer: *mut *mut c_void, values_buffer: *mut *mut c_void, buffer_length: usize) -> usize {
    let content = Box::leak(RustStringRange::from_void(content as *mut _));
    let search_key = Box::leak(RustStringRange::from_void(search_key as *mut _));

    let result = find_all_internal(content.as_slice(), search_key.as_slice(), is_text_search, is_wildcard_search);

    let len = std::cmp::min(result.len(), buffer_length);
    for i in 0..len {
        *keys_buffer.offset(i as isize) = Box::into_raw(Box::new(RustStringRange::from_str(result[i].0))) as *mut c_void;
        *values_buffer.offset(i as isize) = Box::into_raw(Box::new(RustStringRange::from_str(result[i].1))) as *mut c_void;
    }

    len
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

    mod find_worker {
        use super::super::*;

        #[test]
        fn find() {
            let input = "\"A\"=\"錒\"\n\"AES\"=\"厑\"\n\"AI\"=\"爱\"";
            let vec = find_all_internal(input, "ai", false, false);
            assert_eq!(vec, [("AI", "爱")]);
        }

        #[test]
        fn find_value() {
            let input = "\"A\"=\"錒\"\n\"AES\"=\"厑\"";
            let input_full = format!("{}{}", input, "\n\"AI\"=\"爱\"");
            let vec = find_all_internal(&input_full[..], "厑", true, false);
            assert_eq!(vec, [("AES", "厑")]);
        }

        #[test]
        fn find_crlf() {
            let input = "\"A\"=\"錒\"\r\n\"AES\"=\"厑\"\r\n\"AI\"=\"爱\"";
            let vec = find_all_internal(input, "ai", false, false);
            assert_eq!(vec, [("AI", "爱")]);
        }

        #[test]
        fn find_all_wildcard() {
            let input = "\"A\"=\"錒\"\n\"AES\"=\"厑\"\n\"AI\"=\"爱\"\n\"AI\"=\"矮\"";
            let vec = find_all_internal(input, "ai", false, true);
            assert_eq!(vec, [("AI", "爱"), ("AI", "矮")]);
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
