#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

extern HINSTANCE DLL_INSTANCE;

bool register_categories();

bool register_profile(HINSTANCE dll_instance_handle);

bool register_server(HINSTANCE dll_instance_handle);

} // extern "C"
