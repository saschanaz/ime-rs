use core::ffi::c_void;
use ruststringrange::RustStringRange;

mod parser;
use parser::find_items;

mod engine;
use engine::TableDictionaryEngine;

unsafe fn tuples_to_ffi(
    tuples: Vec<(&str, &str)>,
    keys_buffer: *mut *mut c_void,
    values_buffer: *mut *mut c_void,
    buffer_length: usize,
) -> usize {
    let len = std::cmp::min(tuples.len(), buffer_length);
    for i in 0..len {
        *keys_buffer.offset(i as isize) =
            Box::into_raw(Box::new(RustStringRange::from_str(tuples[i].0))) as *mut c_void;
        *values_buffer.offset(i as isize) =
            Box::into_raw(Box::new(RustStringRange::from_str(tuples[i].1))) as *mut c_void;
    }

    len
}

#[no_mangle]
pub unsafe extern "C" fn dictionary_find_items(
    content: *const c_void,
    search_key: *const c_void,
    is_text_search: bool,
    is_wildcard_search: bool,
    keys_buffer: *mut *mut c_void,
    values_buffer: *mut *mut c_void,
    buffer_length: usize,
) -> usize {
    let content = Box::leak(RustStringRange::from_void(content as *mut _));
    let search_key = Box::leak(RustStringRange::from_void(search_key as *mut _));

    let result = find_items(
        content.as_slice(),
        search_key.as_slice(),
        is_text_search,
        is_wildcard_search,
    );

    tuples_to_ffi(result, keys_buffer, values_buffer, buffer_length)
}

#[no_mangle]
pub unsafe extern "C" fn tabledictionaryengine_load(
    path: *const c_void,
    sort: bool,
) -> *mut c_void {
    let path = Box::leak(RustStringRange::from_void(path as *mut _));
    let result = TableDictionaryEngine::load(path.as_slice(), sort);
    if result.is_err() {
        0 as *mut c_void
    } else {
        Box::into_raw(Box::new(result.unwrap())) as *mut c_void
    }
}

#[no_mangle]
pub unsafe extern "C" fn tabledictionaryengine_free(engine: *mut c_void) {
    TableDictionaryEngine::from_void(engine); // implicit cleanup
}

#[no_mangle]
pub unsafe extern "C" fn tabledictionaryengine_collect_word(
    engine: *const c_void,
    search_key: *const c_void,
    keys_buffer: *mut *mut c_void,
    values_buffer: *mut *mut c_void,
    buffer_length: usize,
) -> usize {
    let engine = Box::leak(TableDictionaryEngine::from_void(engine as *mut _));
    let search_key = Box::leak(RustStringRange::from_void(search_key as *mut _));

    let result = engine.collect_word(search_key.as_slice());

    tuples_to_ffi(result, keys_buffer, values_buffer, buffer_length)
}

#[no_mangle]
pub unsafe extern "C" fn tabledictionaryengine_collect_word_for_wildcard(
    engine: *const c_void,
    search_key: *const c_void,
    keys_buffer: *mut *mut c_void,
    values_buffer: *mut *mut c_void,
    buffer_length: usize,
) -> usize {
    let engine = Box::leak(TableDictionaryEngine::from_void(engine as *mut _));
    let search_key = Box::leak(RustStringRange::from_void(search_key as *mut _));

    let result = engine.collect_word_for_wildcard(search_key.as_slice());

    tuples_to_ffi(result, keys_buffer, values_buffer, buffer_length)
}

#[no_mangle]
pub unsafe extern "C" fn tabledictionaryengine_collect_word_from_converted_string_for_wildcard(
    engine: *const c_void,
    search_key: *const c_void,
    keys_buffer: *mut *mut c_void,
    values_buffer: *mut *mut c_void,
    buffer_length: usize,
) -> usize {
    let engine = Box::leak(TableDictionaryEngine::from_void(engine as *mut _));
    let search_key = Box::leak(RustStringRange::from_void(search_key as *mut _));

    let result = engine.collect_word_from_converted_string_for_wildcard(search_key.as_slice());

    tuples_to_ffi(result, keys_buffer, values_buffer, buffer_length)
}
