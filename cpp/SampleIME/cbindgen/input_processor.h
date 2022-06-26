#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <minwindef.h>

extern "C" {

bool is_keyboard_disabled(ITfThreadMgr* thread_mgr, uint32_t tf_client_id);

} // extern "C"
