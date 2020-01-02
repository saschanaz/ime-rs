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
    fn utf16(s: &str) -> Vec<u16> {
        s.encode_utf16().collect()
    }

    #[test]
    fn equalsign() {
        let vec = utf16("abc=");
        let ptr = vec.as_ptr();

        unsafe {
            let result = crate::get_equalsign(ptr, vec.len());
            assert_eq!(result, ptr.offset(3) as *mut u16);
            let char_code = *result as u16;
            assert_eq!(char_code, b'='.into())
        }
    }

    #[test]
    fn equalsign_wrapped() {
        let vec = utf16("\"abc=\"=");
        let ptr = vec.as_ptr();

        unsafe {
            let result = crate::get_equalsign(ptr, vec.len());
            assert_eq!(result, ptr.offset(6) as *mut u16);
            let char_code = *result as u16;
            assert_eq!(char_code, b'='.into())
        }
    }

    #[test]
    fn equalsign_wrapped_nomatch() {
        let vec = utf16("\"abc=\"");
        let ptr = vec.as_ptr();

        unsafe {
            let result = crate::get_equalsign(ptr, vec.len());
            assert_eq!(result, 0 as *mut u16);
        }
    }
}
