#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

bool register_server(HINSTANCE dll_instance_handle);

void unregister_server();

} // extern "C"
