#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <minwindef.h>

extern "C" {

bool init_key_event_sink(ITfThreadMgr* thread_mgr, uint32_t tf_client_id, ITfKeyEventSink* sink);

bool is_key_eaten(ITfThreadMgr* thread_mgr,
                  uint32_t tf_client_id,
                  const void *engine,
                  bool composing,
                  CandidateMode candidate_mode,
                  uint32_t code,
                  uint16_t *ch,
                  KeystrokeCategory *category,
                  KeystrokeFunction *function);

bool on_key_up(ITfThreadMgr* thread_mgr,
               uint32_t tf_client_id,
               void *engine,
               bool composing,
               CandidateMode candidate_mode,
               WPARAM wparam,
               LPARAM lparam);

void uninit_key_event_sink(ITfThreadMgr* thread_mgr, uint32_t tf_client_id);

} // extern "C"
