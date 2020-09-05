#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

uint16_t *get_equalsign(const uint16_t *buffer, uintptr_t buffer_len);

bool parse_line(const void *line, void **key, void **value);

} // extern "C"
