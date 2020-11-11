#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

bool parse_line(const void *line, void **key, void **value);

} // extern "C"
