fn get_equalsign(s: &str) -> Option<usize> {
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

fn parse_line(line: &str) -> Option<(&str, &str)> {
    fn unwrap(text: &str) -> &str {
        let mut result = text;
        if text.starts_with('\"') && text.ends_with('\"') {
            result = &text[1..text.len() - 1];
        }
        result.trim()
    }

    let equalsign = get_equalsign(line);
    equalsign?;

    let key_slice = &line[0..equalsign.unwrap()];
    let value_slice = &line[equalsign.unwrap() + 1..];
    Some((unwrap(key_slice), unwrap(value_slice)))
}

pub fn find_items<'a>(
    content: &'a str,
    search_key: &str,
    is_text_search: bool,
    is_wildcard_search: bool,
) -> Vec<(&'a str, &'a str)> {
    use crate::compare_with_wildcard::compare_with_wildcard;

    let mut vec: Vec<(&'a str, &'a str)> = Vec::new();
    for line in content.lines() {
        let (key, value) = parse_line(line).unwrap();
        let target = if is_text_search { value } else { key };
        let matches = if is_wildcard_search {
            compare_with_wildcard(search_key, target)
        } else {
            search_key.eq_ignore_ascii_case(target)
        };
        if matches {
            vec.push((key, value));
        }
    }
    vec
}

#[cfg(test)]
mod tests {
    mod line_parser {
        use super::super::*;

        #[test]
        fn parse() {
            let line = "\"abc\"=\"bcd\"";
            let (key, value) = parse_line(line).unwrap();
            assert_eq!(key, "abc");
            assert_eq!(value, "bcd");
        }

        #[test]
        fn parse_equalsign_wrapped() {
            let line_raw = "\"a=bc\"=\"bc=d\"";
            let (key, value) = parse_line(line_raw).unwrap();
            assert_eq!(key, "a=bc");
            assert_eq!(value, "bc=d");
        }
    }

    mod find_worker {
        use super::super::*;

        #[test]
        fn find() {
            let input = "\"A\"=\"錒\"\n\"AES\"=\"厑\"\n\"AI\"=\"爱\"";
            let vec = find_items(input, "ai", false, false);
            assert_eq!(vec, [("AI", "爱")]);
        }

        #[test]
        fn find_value() {
            let input = "\"A\"=\"錒\"\n\"AES\"=\"厑\"\n\"AI\"=\"爱\"";
            let vec = find_items(input, "厑", true, false);
            assert_eq!(vec, [("AES", "厑")]);
        }

        #[test]
        fn find_crlf() {
            let input = "\"A\"=\"錒\"\r\n\"AES\"=\"厑\"\r\n\"AI\"=\"爱\"";
            let vec = find_items(input, "ai", false, false);
            assert_eq!(vec, [("AI", "爱")]);
        }

        #[test]
        fn find_all_wildcard() {
            let input = "\"A\"=\"錒\"\n\"AES\"=\"厑\"\n\"AI\"=\"爱\"\n\"AI\"=\"矮\"";
            let vec = find_items(input, "ai", false, true);
            assert_eq!(vec, [("AI", "爱"), ("AI", "矮")]);
        }
    }

    mod equalsign_getter {
        use super::super::*;

        #[test]
        fn equalsign() {
            let s = "abc=";

            let result = get_equalsign(s).unwrap();
            assert_eq!(result, 3);
            let char_code = s.as_bytes().get(result).unwrap();
            assert_eq!(char_code, &b'=');
        }

        #[test]
        fn equalsign_wrapped() {
            let s = "\"abc=\"=";

            let result = get_equalsign(s).unwrap();
            assert_eq!(result, 6);
            let char_code = s.as_bytes().get(result).unwrap();
            assert_eq!(char_code, &b'=');
        }

        #[test]
        fn equalsign_wrapped_nomatch() {
            let s = "\"abc=\"";

            let result = get_equalsign(s);
            assert!(result.is_none());
        }

        #[test]
        fn equalsign_no_equal() {
            let s = "\"abc\"";

            let result = get_equalsign(s);
            assert!(result.is_none());
        }
    }
}
