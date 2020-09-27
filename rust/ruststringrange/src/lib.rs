use std::rc::Rc;
use core::ffi::c_void;
use std::cmp::Ordering;

use compare_with_wildcard::compare_with_wildcard;

// Don't bind the struct itself, instead just expose functions receiving and returning void*
// The C++ wrapper will care those void pointers

#[derive(Clone)]
pub struct RustStringRange {
  string: Rc<String>,
  offset: usize,
  length: usize,
}

impl RustStringRange {
  pub fn len(&self) -> usize {
    self.length
  }

  fn from_string(s: String) -> RustStringRange {
    let string = Rc::new(s);
    let length = string.len();

    RustStringRange {
      string,
      offset: 0,
      length
    }
  }

  pub fn from_str(s: &str) -> RustStringRange {
    let string = String::from(s);
    RustStringRange::from_string(string)
  }

  pub unsafe fn from_buffer_utf16(buffer: *const u16, buffer_len: usize) -> RustStringRange {
    let buffer_slice: &[u16] = std::slice::from_raw_parts(buffer, buffer_len);

    RustStringRange::from_string(String::from_utf16_lossy(buffer_slice))
  }

  pub unsafe fn from_buffer_utf8(buffer: *const u8, buffer_len: usize) -> RustStringRange {
    let buffer_slice: &[u8] = std::slice::from_raw_parts(buffer, buffer_len);

    RustStringRange::from_str(&String::from_utf8_lossy(buffer_slice))
  }

  pub unsafe fn from_void(p: *mut c_void) -> Box<RustStringRange> {
    Box::from_raw(p as *mut RustStringRange)
  }

  pub unsafe fn as_ptr(&self) -> *const u8 {
    self.string.as_ptr().offset(self.offset as isize)
  }

  pub fn cut_last(&self) -> RustStringRange {
    let last = self.as_slice().chars().rev().next();
    if last.is_none() {
      return self.clone();
    }
    RustStringRange {
      string: Rc::clone(&self.string),
      offset: self.offset,
      length: self.length - last.unwrap().len_utf8()
    }
  }

  pub fn as_slice(&self) -> &str {
    &self.string[self.offset..self.offset + self.length]
  }

  pub fn concat(&self, s: &RustStringRange) -> RustStringRange {
    let sum = [self.as_slice(), s.as_slice()].concat();
    let length = sum.len();
    RustStringRange {
      string: Rc::new(sum),
      offset: 0,
      length
    }
  }

  pub fn compare(&self, s: &RustStringRange) -> i8 {
    let order = self.string.cmp(&s.string);
    match order {
      Ordering::Less => -1,
      Ordering::Greater => 1,
      Ordering::Equal => 0
    }
  }

  pub fn contains(&self, s: char) -> bool {
    self.as_slice().contains(s)
  }
}

#[no_mangle]
pub unsafe extern fn ruststringrange_new(buffer: *const u16, buffer_len: usize) -> *mut c_void {
  Box::into_raw(Box::new(RustStringRange::from_buffer_utf16(buffer, buffer_len))) as *mut c_void
}

#[no_mangle]
pub unsafe extern fn ruststringrange_new_utf8(buffer: *const u8, buffer_len: usize) -> *mut c_void {
  Box::into_raw(Box::new(RustStringRange::from_buffer_utf8(buffer, buffer_len))) as *mut c_void
}

#[no_mangle]
pub unsafe extern fn ruststringrange_free(p: *mut c_void) -> () {
  RustStringRange::from_void(p); // implicit cleanup
}

#[no_mangle]
pub unsafe extern fn ruststringrange_raw(p: *mut c_void) -> *const u8 {
  let rsr = Box::leak(RustStringRange::from_void(p));
  rsr.as_ptr()
}

#[no_mangle]
pub unsafe extern fn ruststringrange_len(p: *const c_void) -> usize {
  let rsr = Box::leak(RustStringRange::from_void(p as *mut c_void));
  rsr.len()
}

#[no_mangle]
pub unsafe extern fn ruststringrange_compare(x_raw: *mut c_void, y_raw: *mut c_void) -> i8 {
  let x = Box::leak(RustStringRange::from_void(x_raw));
  let y = Box::leak(RustStringRange::from_void(y_raw));
  x.compare(y)
}

#[no_mangle]
pub unsafe extern fn ruststringrange_compare_with_wildcard(x_raw: *mut c_void, y_raw: *mut c_void) -> bool {
  let x = Box::leak(RustStringRange::from_void(x_raw));
  let y = Box::leak(RustStringRange::from_void(y_raw));
  compare_with_wildcard(&x.as_slice(), &y.as_slice())
}

#[no_mangle]
pub unsafe extern fn ruststringrange_clone(p: *const c_void) -> *mut c_void {
  let rsr = Box::leak(RustStringRange::from_void(p as *mut c_void));
  Box::into_raw(Box::new(rsr.clone())) as *mut c_void
}

#[no_mangle]
pub unsafe extern fn ruststringrange_concat(p1: *const c_void, p2: *const c_void) -> *mut c_void {
  let x = Box::leak(RustStringRange::from_void(p1 as *mut c_void));
  let y = Box::leak(RustStringRange::from_void(p2 as *mut c_void));

  Box::into_raw(Box::new(x.concat(y))) as *mut c_void
}

#[no_mangle]
pub unsafe extern fn ruststringrange_contains(p: *const c_void, ch: u8) -> bool {
  let sr = Box::leak(RustStringRange::from_void(p as *mut c_void));

  sr.contains(char::from(ch))
}

#[no_mangle]
pub unsafe extern fn ruststringrange_cutlast(p: *mut c_void) -> *mut c_void {
  let sr = Box::leak(RustStringRange::from_void(p as *mut c_void));

  Box::into_raw(Box::new(sr.cut_last())) as *mut c_void
}
