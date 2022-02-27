#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

extern "C" {

extern HINSTANCE DLL_INSTANCE;

HICON get_icon(int32_t desired_size, uint32_t index);

} // extern "C"
