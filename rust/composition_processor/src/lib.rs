#![allow(clippy::missing_safety_doc)]

use core::ffi::c_void;
use dictionary_parser::TableDictionaryEngine;
use ruststringrange::RustStringRange;
use winapi::shared::minwindef::HINSTANCE;

mod engine;
use engine::CompositionProcessorEngine;

#[no_mangle]
pub extern "C" fn compositionprocessorengine_new() -> *mut c_void {
    Box::into_raw(Box::new(CompositionProcessorEngine::new())) as *mut c_void
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_free(engine: *mut c_void) {
    CompositionProcessorEngine::from_void(engine); // implicit cleanup
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_add_virtual_key(
    engine: *mut c_void,
    wch: u16,
) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.add_virtual_key(wch)
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_pop_virtual_key(engine: *mut c_void) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.pop_virtual_key();
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_purge_virtual_key(engine: *mut c_void) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.purge_virtual_key();
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_has_virtual_key(engine: *mut c_void) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.has_virtual_key()
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_get_reading_string(
    engine: *mut c_void,
) -> *mut c_void {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    let s = engine.get_reading_string();
    Box::into_raw(Box::new(RustStringRange::from_str(&s))) as *mut c_void
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_keystroke_buffer_includes_wildcard(
    engine: *mut c_void,
) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.keystroke_buffer_includes_wildcard()
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_setup_dictionary_file(
    engine: *mut c_void,
    dll_instance_handle: HINSTANCE,
    dictionary_file_name: *mut c_void,
    is_keystroke_sort: bool,
) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    let dictionary_file_name = Box::leak(RustStringRange::from_void(dictionary_file_name));
    engine.setup_dictionary_file(
        dll_instance_handle,
        dictionary_file_name.as_slice(),
        is_keystroke_sort,
    );
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_get_table_dictionary_engine(
    engine: *const c_void,
) -> *const c_void {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    let dict = engine.get_table_dictionary_engine();
    if dict.is_none() {
        std::ptr::null_mut()
    } else {
        dict.as_ref().unwrap() as *const TableDictionaryEngine as *const c_void
    }
}
