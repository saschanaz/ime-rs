#![allow(clippy::missing_safety_doc)]

use core::ffi::c_void;
use dictionary_parser::TableDictionaryEngine;
use ruststringrange::RustStringRange;

use windows::{
    core::{GUID, HRESULT},
    Win32::{
        Foundation::{LPARAM, WPARAM},
        UI::TextServices::ITfThreadMgr,
    },
};

mod engine;
pub use engine::CompositionProcessorEngine;

mod test_virtual_key;
pub use test_virtual_key::{CandidateMode, KeystrokeCategory, KeystrokeFunction};

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
pub unsafe extern "C" fn compositionprocessorengine_add_virtual_key(
    engine: *mut c_void,
    wch: u16,
) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.keystroke_buffer_mut().add_virtual_key(wch)
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_pop_virtual_key(engine: *mut c_void) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.keystroke_buffer_mut().pop_virtual_key();
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_purge_virtual_key(engine: *mut c_void) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.keystroke_buffer_mut().purge_virtual_key();
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_has_virtual_key(engine: *mut c_void) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.keystroke_buffer().has_virtual_key()
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_keystroke_buffer_get_reading_string(
    engine: *mut c_void,
) -> *mut c_void {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    let s = engine.keystroke_buffer().get_reading_string();
    Box::into_raw(Box::new(RustStringRange::from_str(&s))) as *mut c_void
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_keystroke_buffer_includes_wildcard(
    engine: *mut c_void,
) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine));
    engine.keystroke_buffer().includes_wildcard()
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

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_compartmentwrapper_raw_ptr(
    engine: *mut c_void,
) -> *const c_void {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    engine.compartment_wrapper() as *const _ as *const c_void
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_setup_language_profile(
    engine: *mut c_void,
    thread_mgr: ITfThreadMgr,
    client_id: u32,
) -> bool {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    engine.setup_language_profile(thread_mgr, client_id)
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_hide_language_bar_button(
    engine: *mut c_void,
    hide: bool,
) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    engine.hide_language_bar_button(hide).ok();
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_disable_language_bar_button(
    engine: *mut c_void,
    disable: bool,
) {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    engine.disable_language_bar_button(disable).ok();
}

unsafe fn tuples_to_ffi(
    tuples: Vec<(&str, &str)>,
    keys_buffer: *mut *mut c_void,
    values_buffer: *mut *mut c_void,
    buffer_length: usize,
) -> usize {
    let len = std::cmp::min(tuples.len(), buffer_length);
    for (i, tuple) in tuples.iter().enumerate().take(len) {
        *keys_buffer.add(i) =
            Box::into_raw(Box::new(RustStringRange::from_str(tuple.0))) as *mut c_void;
        *values_buffer.add(i) =
            Box::into_raw(Box::new(RustStringRange::from_str(tuple.1))) as *mut c_void;
    }

    len
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_get_candidate_list(
    engine: *const c_void,
    keys_buffer: *mut *mut c_void,
    values_buffer: *mut *mut c_void,
    buffer_length: usize,
    is_incremental_word_search: bool,
    is_wildcard_search: bool,
) -> usize {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));

    let result = engine.get_candidate_list(is_incremental_word_search, is_wildcard_search);

    tuples_to_ffi(result, keys_buffer, values_buffer, buffer_length)
}

#[no_mangle]
pub unsafe extern "C" fn compositionprocessorengine_get_candidate_string_in_converted(
    engine: *const c_void,
    search_key: *const c_void,
    keys_buffer: *mut *mut c_void,
    values_buffer: *mut *mut c_void,
    buffer_length: usize,
) -> usize {
    let engine = Box::leak(CompositionProcessorEngine::from_void(engine as *mut _));
    let search_key = Box::leak(RustStringRange::from_void(search_key as *mut _));

    let result = engine.get_candidate_string_in_converted(search_key.as_slice());

    tuples_to_ffi(result, keys_buffer, values_buffer, buffer_length)
}
