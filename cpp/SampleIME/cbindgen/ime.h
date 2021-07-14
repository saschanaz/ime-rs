#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

bool register_categories();

bool register_server(HINSTANCE dll_instance_handle);

bool unregister_categories();

void unregister_server();

} // extern "C"
