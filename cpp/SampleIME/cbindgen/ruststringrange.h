#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

extern "C" {

void *ruststringrange_new(const uint16_t *buffer, uintptr_t buffer_len);

void *ruststringrange_new_utf8(const uint8_t *buffer, uintptr_t buffer_len);

void ruststringrange_free(void *p);

const uint8_t *ruststringrange_raw(void *p);

uintptr_t ruststringrange_len(const void *p);

int8_t ruststringrange_compare(void *x_raw, void *y_raw);

void *ruststringrange_clone(const void *p);

} // extern "C"
