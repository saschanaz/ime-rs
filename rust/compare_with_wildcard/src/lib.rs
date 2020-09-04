pub fn compare_with_wildcard(input: &str, target: &str) -> bool {
    if input.len() == 0 {
        return target.len() == 0;
    }

    let input_char = input.chars().next().unwrap();
    let input_charlen = input_char.len_utf8();

    let target_char = target.chars().next();
    let target_charlen = target_char.map(|c| c.len_utf8());

    if input_char == '*' {
        // 1. skip this wildcard and compare remaining characters
        // 2. recursively retry with target[1..]
        if compare_with_wildcard(&input[input_charlen..], target) {
            return true;
        }
        if target.len() == 0 {
            return false;
        }
        return compare_with_wildcard(input, &target[target_charlen.unwrap()..]);
    }

    if target.len() == 0 {
        return false;
    }

    if input_char == '?' {
        // skip this mark and compare with target[1..]
        return compare_with_wildcard(&input[input_charlen..], &target[target_charlen.unwrap()..]);
    }

    if !input_char.eq_ignore_ascii_case(&target_char.unwrap()) {
        return false;
    }

    compare_with_wildcard(&input[input_charlen..], &target[target_charlen.unwrap()..])
}

#[no_mangle]
pub unsafe extern fn compare_with_wildcard_utf16(input_buffer: *const u16, input_len: usize, target_buffer: *const u16, target_len: usize) -> bool {

    let input_slice: &[u16] = std::slice::from_raw_parts(input_buffer, input_len);
    let target_slice: &[u16] = std::slice::from_raw_parts(target_buffer, target_len);

    let input = String::from_utf16_lossy(input_slice);
    let target = String::from_utf16_lossy(target_slice);

    compare_with_wildcard(&input, &target)
}

#[cfg(test)]
mod tests {
    fn utf16(s: &str) -> Vec<u16> {
        s.encode_utf16().collect()
    }

    unsafe fn wrapped_compare(input: &str, reference: &str) -> bool {
        let input_enc = utf16(input);
        let reference_enc = utf16(reference);

        crate::compare_with_wildcard_utf16(
            input_enc.as_ptr(),
            input_enc.len(),
            reference_enc.as_ptr(),
            reference_enc.len()
        )
    }

    #[test]
    fn compare_empty() {
        unsafe {
            assert_eq!(wrapped_compare("", ""), true);
        }
    }

    #[test]
    fn compare_empty_filled() {
        unsafe {
            assert_eq!(wrapped_compare("", "s"), false);
        }
    }

    #[test]
    fn compare_filled_empty() {
        unsafe {
            assert_eq!(wrapped_compare("s", ""), false);
        }
    }

    #[test]
    fn compare_filled_same() {
        unsafe {
            assert_eq!(wrapped_compare("wow", "wow"), true);
            assert_eq!(wrapped_compare("wow", "wOw"), true);
            assert_eq!(wrapped_compare("사과", "사과"), true);
        }
    }

    #[test]
    fn compare_filled_different() {
        unsafe {
            assert_eq!(wrapped_compare("wow", "cow"), false);
            assert_eq!(wrapped_compare("wow", "wot"), false);
            assert_eq!(wrapped_compare("사과", "수박"), false);
        }
    }

    #[test]
    fn compare_different_length() {
        unsafe {
            assert_eq!(wrapped_compare("wowwow", "wow"), false);
            assert_eq!(wrapped_compare("what", "what?"), false);
            assert_eq!(wrapped_compare("사과", "귤"), false);
            assert_eq!(wrapped_compare("사과", "바나나"), false);
        }
    }

    #[test]
    fn compare_wildcard() {
        unsafe {
            assert_eq!(wrapped_compare("w*", "w"), true);
            assert_eq!(wrapped_compare("w*", "wo"), true);
            assert_eq!(wrapped_compare("w*", "wow"), true);
            assert_eq!(wrapped_compare("w*t", "wat"), true);
            assert_eq!(wrapped_compare("w*t", "what"), true);
            assert_eq!(wrapped_compare("w*t", "wha"), false);
            assert_eq!(wrapped_compare("사*", "사과"), true);
        }
    }

    #[test]
    fn compare_exclamation() {
        unsafe {
            assert_eq!(wrapped_compare("w?", "wo"), true);
            assert_eq!(wrapped_compare("w?", "wow"), false);
            assert_eq!(wrapped_compare("w?t", "wat"), true);
            assert_eq!(wrapped_compare("w?t", "what"), false);
            assert_eq!(wrapped_compare("w?t", "wha"), false);
            assert_eq!(wrapped_compare("사?", "사과"), true);
        }
    }
}
