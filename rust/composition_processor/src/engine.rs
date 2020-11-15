use dictionary_parser::TableDictionaryEngine;

use winapi::shared::minwindef::{HINSTANCE, MAX_PATH};
use winapi::um::libloaderapi::GetModuleFileNameW;

pub struct CompositionProcessorEngine {
    table_dictionary_engine: Option<TableDictionaryEngine>,
}

impl CompositionProcessorEngine {
    pub fn new() -> CompositionProcessorEngine {
        CompositionProcessorEngine {
            table_dictionary_engine: None,
        }
    }

    pub unsafe fn from_void(engine: *mut std::ffi::c_void) -> Box<CompositionProcessorEngine> {
        Box::from_raw(engine as *mut CompositionProcessorEngine)
    }

    pub fn setup_dictionary_file(
        &mut self,
        dll_instance_handle: HINSTANCE,
        dictionary_file_name: &str,
        is_keystroke_sort: bool,
    ) {
        let mut file_name: [u16; MAX_PATH] = [0; MAX_PATH];
        unsafe {
            GetModuleFileNameW(dll_instance_handle, file_name.as_mut_ptr(), MAX_PATH as u32);
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
}

// #[cfg(test)]
// mod tests {
//     #[test]
//     fn it_works() {
//         assert_eq!(2 + 2, 4);
//     }
// }
