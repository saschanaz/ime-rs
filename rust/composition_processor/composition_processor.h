#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <minwindef.h>

extern "C" {

bool compositionprocessorengine_add_virtual_key(void *engine, uint16_t wch);

void compositionprocessorengine_free(void *engine);

void *compositionprocessorengine_get_reading_string(void *engine);

const void *compositionprocessorengine_get_table_dictionary_engine(const void *engine);

bool compositionprocessorengine_has_virtual_key(void *engine);

bool compositionprocessorengine_keystroke_buffer_includes_wildcard(void *engine);

void *compositionprocessorengine_new();

void compositionprocessorengine_pop_virtual_key(void *engine);

void compositionprocessorengine_purge_virtual_key(void *engine);

void compositionprocessorengine_setup_dictionary_file(void *engine,
                                                      HINSTANCE dll_instance_handle,
                                                      void *dictionary_file_name,
                                                      bool is_keystroke_sort);

} // extern "C"
