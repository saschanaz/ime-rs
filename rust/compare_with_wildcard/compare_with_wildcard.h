#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

bool compare_with_wildcard_utf16(const uint16_t *input_buffer,
                                 uintptr_t input_len,
                                 const uint16_t *target_buffer,
                                 uintptr_t target_len);

} // extern "C"