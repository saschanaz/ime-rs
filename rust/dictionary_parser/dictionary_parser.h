#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

uintptr_t dictionary_find_items(const void *content,
                                const void *search_key,
                                bool is_text_search,
                                bool is_wildcard_search,
                                void **keys_buffer,
                                void **values_buffer,
                                uintptr_t buffer_length);

} // extern "C"
