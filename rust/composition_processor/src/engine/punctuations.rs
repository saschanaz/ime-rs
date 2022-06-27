use std::collections::HashMap;

struct QuotePunctuation {
    full_width_open: char,
    full_width_close: char,
    toggle: bool,
}

impl QuotePunctuation {
    fn get_and_toggle(&mut self) -> char {
        let alternative = if !self.toggle {
            self.full_width_open
        } else {
            self.full_width_close
        };
        self.toggle = !self.toggle;
        alternative
    }
}

struct AngleBracketPunctuation {
    half_width: char,
    full_width_outer: char,
    full_width_inner: char,
}

impl AngleBracketPunctuation {
    fn get_by_count(&self, nested_count: i16) -> char {
        if nested_count == 0 {
            self.full_width_outer
        } else {
            self.full_width_inner
        }
    }
}

struct AngleBracketsPunctuation {
    open: AngleBracketPunctuation,
    close: AngleBracketPunctuation,
    /** Not a strict nested count, it just increments when opening and decrements when closing */
    nested_count: i16,
}

impl AngleBracketsPunctuation {
    fn is_angle_bracket(&self, c: char) -> bool {
        c == self.open.half_width || c == self.close.half_width
    }
    fn get_and_count_by(&mut self, c: char) -> Option<char> {
        if c == self.open.half_width {
            let result = self.open.get_by_count(self.nested_count);
            self.nested_count += 1;
            Some(result)
        } else if c == self.close.half_width {
            self.nested_count -= 1;
            Some(self.close.get_by_count(self.nested_count))
        } else {
            None
        }
    }
}

pub struct PunctuationMapper {
    punctuation_table: HashMap<char, char>,
    quotes: HashMap<char, QuotePunctuation>,
    angle_brackets: AngleBracketsPunctuation,
}

impl PunctuationMapper {
    pub fn has_alternative_punctuation(&self, punctuation: char) -> bool {
        self.punctuation_table.contains_key(&punctuation)
            || self.quotes.contains_key(&punctuation)
            || self.angle_brackets.is_angle_bracket(punctuation)
    }

    pub fn get_alternative_punctuation_counted(&mut self, punctuation: char) -> char {
        if let Some(alternative) = self.punctuation_table.get(&punctuation) {
            return alternative.to_owned();
        }
        if let Some(quote) = self.quotes.get_mut(&punctuation) {
            return quote.get_and_toggle();
        }
        if let Some(alternative) = self.angle_brackets.get_and_count_by(punctuation) {
            return alternative.to_owned();
        }

        '\0'
    }
}

impl Default for PunctuationMapper {
    fn default() -> PunctuationMapper {
        let punctuation_table: HashMap<char, char> = {
            let mut map = HashMap::new();
            map.insert('!', '\u{FF01}');
            map.insert('$', '\u{FFE5}');
            map.insert('&', '\u{2014}');
            map.insert('(', '\u{FF08}');
            map.insert(')', '\u{FF09}');
            map.insert(',', '\u{FF0C}');
            map.insert('.', '\u{3002}');
            map.insert(':', '\u{FF1A}');
            map.insert(';', '\u{FF1B}');
            map.insert('?', '\u{FF1F}');
            map.insert('@', '\u{00B7}');
            map.insert('\\', '\u{3001}');
            map.insert('^', '\u{2026}');
            map.insert('_', '\u{2014}');
            map
        };

        let quotes: HashMap<char, QuotePunctuation> = {
            let mut map = HashMap::new();
            map.insert(
                '\'',
                QuotePunctuation {
                    full_width_open: '‘',
                    full_width_close: '’',
                    toggle: false,
                },
            );
            map.insert(
                '"',
                QuotePunctuation {
                    full_width_open: '“',
                    full_width_close: '”',
                    toggle: false,
                },
            );

            map
        };

        let angle_brackets = AngleBracketsPunctuation {
            open: AngleBracketPunctuation {
                half_width: '<',
                full_width_outer: '《',
                full_width_inner: '〈',
            },
            close: AngleBracketPunctuation {
                half_width: '>',
                full_width_outer: '》',
                full_width_inner: '〉',
            },
            nested_count: 0,
        };

        PunctuationMapper {
            punctuation_table,
            quotes,
            angle_brackets,
        }
    }
}
