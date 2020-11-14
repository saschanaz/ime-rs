use core::ffi::c_void;
use ruststringrange::RustStringRange;

mod parser;
use parser::find_items;

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

  let len = std::cmp::min(result.len(), buffer_length);
  for i in 0..len {
    *keys_buffer.offset(i as isize) =
      Box::into_raw(Box::new(RustStringRange::from_str(result[i].0))) as *mut c_void;
    *values_buffer.offset(i as isize) =
      Box::into_raw(Box::new(RustStringRange::from_str(result[i].1))) as *mut c_void;
  }

  len
}
