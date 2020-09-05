#pragma once

#include "..\..\rust\ruststringrange\ruststringrange.h"

#include "SampleIMEBaseStructure.h"

/**
 * A C++ wrapper class for its Rust counterpart
 */
class CRustStringRange {
 public:
  CRustStringRange(void* range_raw) {
    range = range_raw;
  }
  explicit CRustStringRange(const CStringRangeBase& cstr) {
    Set(cstr);
  }
  CRustStringRange(const wchar_t* pwch, uintptr_t dwLength) {
    Set(pwch, dwLength);
  }
  void Set(const wchar_t* pwch, uintptr_t dwLength) {
    range = ruststringrange_new((const uint16_t*)pwch, dwLength);
  }
  void Set(const CStringRangeBase& cstr) {
    Set(cstr.GetRaw(), cstr.GetLength());
  }

  ~CRustStringRange() {
    ruststringrange_free(range);
  }

  uintptr_t GetLengthUtf8() const {
    return ruststringrange_len(range);
  }

  const uint8_t* GetRawUtf8() const {
    return ruststringrange_raw(range);
  }

  void* GetInternal() const {
    return range;
  }

  bool CompareWithWildCard(const CRustStringRange& target) const {
    return ruststringrange_compare_with_wildcard(this->range, target.range);
  }

private:
  void* range = nullptr;
};
