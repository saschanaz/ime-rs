#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

bool register_categories();

bool register_profile(HINSTANCE dll_instance_handle);

bool register_server(HINSTANCE dll_instance_handle);

bool unregister_categories();

bool unregister_profile();

void unregister_server();

} // extern "C"
