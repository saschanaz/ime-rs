#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

bool register_server(void *module_file_name);

void unregister_server();

} // extern "C"
