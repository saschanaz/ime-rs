#include "RustStringRange.h"

CRustStringRange operator""_rs(const char* aStr, std::size_t aLen) {
  return CRustStringRange(aStr, aLen);
}
