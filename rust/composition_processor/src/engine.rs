use dictionary_parser::TableDictionaryEngine;

use crate::bindings::Windows::Win32::Foundation::{HINSTANCE, MAX_PATH, PWSTR};
use crate::bindings::Windows::Win32::System::LibraryLoader::GetModuleFileNameW;
use crate::modifiers::Modifiers;

pub struct CompositionProcessorEngine {
    keystroke_buffer: Vec<u16>,
    table_dictionary_engine: Option<TableDictionaryEngine>,
    modifiers: Modifiers,
}

impl CompositionProcessorEngine {
    pub fn new() -> CompositionProcessorEngine {
        CompositionProcessorEngine {
            keystroke_buffer: Vec::new(),
            table_dictionary_engine: None,
            modifiers: Modifiers::default(),
        }
    }

    pub unsafe fn from_void(engine: *mut std::ffi::c_void) -> Box<CompositionProcessorEngine> {
        Box::from_raw(engine as *mut CompositionProcessorEngine)
    }

    pub fn add_virtual_key(&mut self, wch: u16) -> bool {
        if wch == 0 {
            return false;
        }
        self.keystroke_buffer.push(wch);
        true
    }

    pub fn pop_virtual_key(&mut self) {
        if self.keystroke_buffer.is_empty() {
            return;
        }
        self.keystroke_buffer.pop();
    }

    pub fn purge_virtual_key(&mut self) {
        self.keystroke_buffer.clear();
    }

    pub fn has_virtual_key(&self) -> bool {
        !self.keystroke_buffer.is_empty()
    }

    pub fn get_reading_string(&self) -> String {
        String::from_utf16(&self.keystroke_buffer).unwrap()
    }

    pub fn keystroke_buffer_includes_wildcard(&self) -> bool {
        self.keystroke_buffer.contains(&(b'*' as u16))
            || self.keystroke_buffer.contains(&(b'?' as u16))
    }

    pub fn setup_dictionary_file(
        &mut self,
        dll_instance_handle: HINSTANCE,
        dictionary_file_name: &str,
        is_keystroke_sort: bool,
    ) {
        let mut file_name = [0u16; MAX_PATH as usize];
        unsafe {
            GetModuleFileNameW(dll_instance_handle, PWSTR(file_name.as_mut_ptr()), MAX_PATH);
        }

        let file_name = String::from_utf16(&file_name).unwrap();
        let dir = std::path::Path::new(&file_name[..]).parent().unwrap();
        let dict_path = dir.join(dictionary_file_name);

        self.table_dictionary_engine = Some(
            TableDictionaryEngine::load(dict_path.to_str().unwrap(), is_keystroke_sort).unwrap(),
        )
    }

    pub fn get_table_dictionary_engine(&self) -> &Option<TableDictionaryEngine> {
        &self.table_dictionary_engine
    }

    pub fn modifiers(&mut self) -> &mut Modifiers {
        &mut self.modifiers
    }
}

#[cfg(test)]
mod tests {
    mod virtual_key {
        use super::super::CompositionProcessorEngine;
        #[test]
        fn add_virtual_key() {
            let mut engine = CompositionProcessorEngine::new();
            engine.add_virtual_key(0xce74);
            engine.add_virtual_key(0xac00);
            engine.add_virtual_key(0xbbf8);
            assert_eq!(engine.get_reading_string(), "카가미");
        }

        #[test]
        fn pop_virtual_key() {
            let mut engine = CompositionProcessorEngine::new();
            engine.add_virtual_key(0xb8e8);
            engine.add_virtual_key(0xc2e4);
            engine.add_virtual_key(0xb9ac);
            engine.add_virtual_key(0xce74);
            engine.pop_virtual_key();
            engine.pop_virtual_key();
            assert_eq!(engine.get_reading_string(), "루실");
        }

        #[test]
        fn purge_virtual_key() {
            let mut engine = CompositionProcessorEngine::new();
            engine.add_virtual_key(0xce74);
            engine.pop_virtual_key();
            assert_eq!(engine.get_reading_string(), "");
        }

        #[test]
        fn has_virtual_key() {
            let mut engine = CompositionProcessorEngine::new();
            assert!(!engine.has_virtual_key());

            engine.add_virtual_key(0xce74);
            assert!(engine.has_virtual_key());

            engine.pop_virtual_key();
            assert!(!engine.has_virtual_key());
        }
    }
}
