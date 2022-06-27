#[derive(Default)]
pub struct KeystrokeBuffer {
    buffer: Vec<u16>,
}

impl KeystrokeBuffer {
    pub fn add_virtual_key(&mut self, wch: u16) -> bool {
        if wch == 0 {
            return false;
        }
        self.buffer.push(wch);
        true
    }

    pub fn pop_virtual_key(&mut self) {
        if self.buffer.is_empty() {
            return;
        }
        self.buffer.pop();
    }

    pub fn purge_virtual_key(&mut self) {
        self.buffer.clear();
    }

    pub fn has_virtual_key(&self) -> bool {
        !self.buffer.is_empty()
    }

    pub fn get_reading_string(&self) -> String {
        String::from_utf16(&self.buffer).unwrap()
    }

    pub fn includes_wildcard(&self) -> bool {
        self.buffer.contains(&(b'*' as u16)) || self.buffer.contains(&(b'?' as u16))
    }
}

#[cfg(test)]
mod tests {
    use super::KeystrokeBuffer;

    #[test]
    fn add_virtual_key() {
        let mut buffer = KeystrokeBuffer::default();
        buffer.add_virtual_key(0xce74);
        buffer.add_virtual_key(0xac00);
        buffer.add_virtual_key(0xbbf8);
        assert_eq!(buffer.get_reading_string(), "카가미");
    }

    #[test]
    fn pop_virtual_key() {
        let mut buffer = KeystrokeBuffer::default();
        buffer.add_virtual_key(0xb8e8);
        buffer.add_virtual_key(0xc2e4);
        buffer.add_virtual_key(0xb9ac);
        buffer.add_virtual_key(0xce74);
        buffer.pop_virtual_key();
        buffer.pop_virtual_key();
        assert_eq!(buffer.get_reading_string(), "루실");
    }

    #[test]
    fn purge_virtual_key() {
        let mut buffer = KeystrokeBuffer::default();
        buffer.add_virtual_key(0xce74);
        buffer.pop_virtual_key();
        assert_eq!(buffer.get_reading_string(), "");
    }

    #[test]
    fn has_virtual_key() {
        let mut buffer = KeystrokeBuffer::default();
        assert!(!buffer.has_virtual_key());

        buffer.add_virtual_key(0xce74);
        assert!(buffer.has_virtual_key());

        buffer.pop_virtual_key();
        assert!(!buffer.has_virtual_key());
    }
}
