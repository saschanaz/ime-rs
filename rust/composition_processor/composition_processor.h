#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <minwindef.h>

extern "C" {

void compositionprocessorengine_free(void *engine);

const void *compositionprocessorengine_get_table_dictionary_engine(const void *engine);

void *compositionprocessorengine_new();

void compositionprocessorengine_setup_dictionary_file(void *engine,
                                                      HINSTANCE dll_instance_handle,
                                                      void *dictionary_file_name,
                                                      bool is_keystroke_sort);

} // extern "C"
