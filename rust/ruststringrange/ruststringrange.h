#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

bool ruststringrange_compare_with_wildcard(void *x_raw, void *y_raw);

void ruststringrange_free(void *p);

uintptr_t ruststringrange_len(const void *p);

void *ruststringrange_new(const uint16_t *buffer, uintptr_t buffer_len);

const uint8_t *ruststringrange_raw(void *p);

} // extern "C"