#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

void *ruststringrange_clone(const void *p);

int8_t ruststringrange_compare(void *x_raw, void *y_raw);

bool ruststringrange_compare_with_wildcard(void *x_raw, void *y_raw);

void *ruststringrange_concat(const void *p1, const void *p2);

void ruststringrange_free(void *p);

uintptr_t ruststringrange_len(const void *p);

void *ruststringrange_new(const uint16_t *buffer, uintptr_t buffer_len);

void *ruststringrange_new_utf8(const uint8_t *buffer, uintptr_t buffer_len);

const uint8_t *ruststringrange_raw(void *p);

} // extern "C"
