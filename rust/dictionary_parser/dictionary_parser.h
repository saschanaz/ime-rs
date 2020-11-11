#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

uintptr_t find_worker(const void *content,
                      const void *search_key,
                      bool is_text_search,
                      bool is_wildcard_search,
                      void **key,
                      void **value);

bool parse_line(const void *line, void **key, void **value);

} // extern "C"
