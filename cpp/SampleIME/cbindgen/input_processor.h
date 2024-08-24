#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include <minwindef.h>

extern "C" {

bool key_event_sink_on_key_up(ITfThreadMgr* thread_mgr,
                              uint32_t tf_client_id,
                              void *engine,
                              bool composing,
                              CandidateMode candidate_mode,
                              WPARAM wparam,
                              LPARAM lparam);

bool key_event_sink_init_key_event_sink(ITfThreadMgr* thread_mgr,
                                        uint32_t tf_client_id,
                                        ITfKeyEventSink* sink);

void key_event_sink_uninit_key_event_sink(ITfThreadMgr* thread_mgr, uint32_t tf_client_id);

bool key_event_sink_is_key_eaten(ITfThreadMgr* thread_mgr,
                                 uint32_t tf_client_id,
                                 const void *engine,
                                 bool composing,
                                 CandidateMode candidate_mode,
                                 uint32_t code,
                                 uint16_t *ch,
                                 KeystrokeCategory *category,
                                 KeystrokeFunction *function);

} // extern "C"
