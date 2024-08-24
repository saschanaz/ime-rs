#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

constexpr static const uint32_t IME_MODE_ON_ICO_INDEX = 21;

constexpr static const uint32_t IME_MODE_OFF_ICO_INDEX = 22;

extern "C" {

extern HMODULE DLL_INSTANCE;

extern HFONT DEFAULT_FONT_HANDLE;

} // extern "C"
