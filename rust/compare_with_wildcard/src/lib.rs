#![allow(clippy::missing_safety_doc)]

pub fn compare_with_wildcard(input: &str, target: &str) -> bool {
    if input.is_empty() {
        return target.is_empty();
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
        if target.is_empty() {
            return false;
        }
        return compare_with_wildcard(input, &target[target_charlen.unwrap()..]);
    }

    if target.is_empty() {
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

#[cfg(test)]
mod tests {
    use super::compare_with_wildcard;

    #[test]
    fn compare_empty() {
        assert!(compare_with_wildcard("", ""));
    }

    #[test]
    fn compare_empty_filled() {
        assert!(!compare_with_wildcard("", "s"));
    }

    #[test]
    fn compare_filled_empty() {
        assert!(!compare_with_wildcard("s", ""));
    }

    #[test]
    fn compare_filled_same() {
        assert!(compare_with_wildcard("wow", "wow"));
        assert!(compare_with_wildcard("wow", "wOw"));
        assert!(compare_with_wildcard("사과", "사과"));
    }

    #[test]
    fn compare_filled_different() {
        assert!(!compare_with_wildcard("wow", "cow"));
        assert!(!compare_with_wildcard("wow", "wot"));
        assert!(!compare_with_wildcard("사과", "수박"));
    }

    #[test]
    fn compare_different_length() {
        assert!(!compare_with_wildcard("wowwow", "wow"));
        assert!(!compare_with_wildcard("what", "what?"));
        assert!(!compare_with_wildcard("사과", "귤"));
        assert!(!compare_with_wildcard("사과", "바나나"));
    }

    #[test]
    fn compare_wildcard() {
        assert!(compare_with_wildcard("w*", "w"));
        assert!(compare_with_wildcard("w*", "wo"));
        assert!(compare_with_wildcard("w*", "wow"));
        assert!(compare_with_wildcard("w*t", "wat"));
        assert!(compare_with_wildcard("w*t", "what"));
        assert!(!compare_with_wildcard("w*t", "wha"));
        assert!(compare_with_wildcard("사*", "사과"));
    }

    #[test]
    fn compare_exclamation() {
        assert!(compare_with_wildcard("w?", "wo"));
        assert!(!compare_with_wildcard("w?", "wow"));
        assert!(compare_with_wildcard("w?t", "wat"));
        assert!(!compare_with_wildcard("w?t", "what"));
        assert!(!compare_with_wildcard("w?t", "wha"));
        assert!(compare_with_wildcard("사?", "사과"));
    }
}
