#![allow(clippy::missing_safety_doc)]

use core::ffi::c_void;
use dictionary_parser::TableDictionaryEngine;
use ruststringrange::RustStringRange;

use windows::{
    core::{GUID, HRESULT},
    Win32::{
        Foundation::{HINSTANCE, LPARAM, WPARAM},
        UI::TextServices::ITfThreadMgr,
    },
};

mod engine;
use engine::CompositionProcessorEngine;

mod test_virtual_key;
use test_virtual_key::{CandidateMode, KeystrokeCategory, KeystrokeFunction};

#[no_mangle]
pub extern "C" fn compositionprocessorengine_new(
    thread_mgr: ITfThreadMgr,
    tf_client_id: u32,
) -> *mut c_void {
    Box::into_raw(Box::new(CompositionProcessorEngine::new(
        thread_mgr,
        tf_client_id,
    ))) as *mut c_void
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_free(engine: *mut c_void) {
    CompositionProcessorEngine::from_void(engine); // implicit cleanup
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_test_virtual_key(
    engine: *mut c_void,
    code: u16,
    ch: u16,
    composing: bool,
    candidate_mode: CandidateMode,
    key_eaten: *mut bool,
    keystroke_category: *mut KeystrokeCategory,
    keystroke_function: *mut KeystrokeFunction,
) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    let (eaten, category, function) = engine.test_virtual_key(
        code,
        char::from_u32(ch as u32).unwrap(),
        composing,
        candidate_mode,
    );
    *key_eaten = eaten;
    *keystroke_category = category;
    *keystroke_function = function;
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_add_virtual_key(
    engine: *mut c_void,
    wch: u16,
) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.virtual_key_manager_mut().add_virtual_key(wch)
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_pop_virtual_key(engine: *mut c_void) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.virtual_key_manager_mut().pop_virtual_key();
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_purge_virtual_key(engine: *mut c_void) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.virtual_key_manager_mut().purge_virtual_key();
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_has_virtual_key(engine: *mut c_void) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.virtual_key_manager().has_virtual_key()
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_get_reading_string(
    engine: *mut c_void,
) -> *mut c_void {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    let s = engine.virtual_key_manager().get_reading_string();
    Box::into_raw(Box::new(RustStringRange::from_str(&s))) as *mut c_void
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_keystroke_buffer_includes_wildcard(
    engine: *mut c_void,
) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine
        .virtual_key_manager()
        .keystroke_buffer_includes_wildcard()
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_setup_dictionary_file(
    engine: *mut c_void,
    dll_instance_handle: HINSTANCE,
    dictionary_file_name: *mut c_void,
) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    let dictionary_file_name = Box::leak(RustStringRange::from_void(dictionary_file_name));
    engine.setup_dictionary_file(dll_instance_handle, dictionary_file_name.as_slice());
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

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_modifiers_update(
    engine: *mut c_void,
    w: WPARAM,
    l: LPARAM,
) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    engine.modifiers_mut().update(w, l);
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_modifiers_is_shift_key_down_only(
    engine: *mut c_void,
) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    engine.modifiers().is_shift_key_down_only()
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_modifiers_is_control_key_down_only(
    engine: *mut c_void,
) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    engine.modifiers().is_control_key_down_only()
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_modifiers_is_alt_key_down_only(
    engine: *mut c_void,
) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    engine.modifiers().is_alt_key_down_only()
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_punctuations_has_alternative_punctuation(
    engine: *mut c_void,
    wch: u16,
) -> bool {
    let c = char::from_u32(wch as u32).unwrap();
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    engine
        .punctuation_mapper_mut()
        .has_alternative_punctuation(c)
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_punctuations_get_alternative_punctuation_counted(
    engine: *mut c_void,
    wch: u16,
) -> u16 {
    let c = char::from_u32(wch as u32).unwrap();
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    let result = engine
        .punctuation_mapper_mut()
        .get_alternative_punctuation_counted(c);
    let mut b = [0; 1];
    result.encode_utf16(&mut b);
    b[0]
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_preserved_keys_init(
    engine: *mut c_void,
    thread_mgr: ITfThreadMgr,
    client_id: u32,
) -> HRESULT {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    HRESULT::from(engine.preserved_keys().init_keys(thread_mgr, client_id))
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_on_preserved_key(
    engine: *mut c_void,
    guid: &GUID,
    out_is_eaten: *mut bool,
    thread_mgr: ITfThreadMgr,
    client_id: u32,
) -> HRESULT {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    let result = engine.on_preserved_key(guid, thread_mgr, client_id);

    *out_is_eaten = *result.as_ref().unwrap_or(&false);

    HRESULT::from(result)
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_compartmentwrapper_conversion_mode_compartment_updated(
    engine: *mut c_void,
    thread_mgr: ITfThreadMgr,
) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    engine
        .compartment_wrapper()
        .conversion_mode_compartment_updated(thread_mgr);
}
