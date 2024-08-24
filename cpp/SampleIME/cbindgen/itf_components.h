#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

using CESCALLBACK = HRESULT(*)(const void *pv, const GUID *guid);

extern "C" {

void *compartment_new(ITfThreadMgr* thread_mgr, uint32_t tf_client_id, const GUID *guid);

void compartment_free(void *compartment);

HRESULT compartment_get_bool(void *compartment, bool *data);

HRESULT compartment_set_bool(void *compartment, bool flag);

HRESULT compartment_get_u32(void *compartment, uint32_t *data);

HRESULT compartment_set_u32(void *compartment, uint32_t data);

HRESULT compartment_clear(void *compartment);

void compartment_guid(void *compartment, GUID *guid);

void *compartmenteventsink_new(CESCALLBACK callback, const void *pv);

HRESULT compartmenteventsink_advise(ITfCompartmentEventSink* sink,
                                    IUnknown* punk,
                                    const GUID *guid);

HRESULT compartmenteventsink_unadvise(ITfCompartmentEventSink* sink);

void *langbaritembutton_new(const GUID *item_guid,
                            void *description,
                            void *tooltip,
                            uint32_t on_icon_index,
                            uint32_t off_icon_index);

HRESULT langbaritembutton_init(ITfLangBarItemButton* button,
                               ITfThreadMgr* thread_mgr,
                               uint32_t tf_client_id,
                               const GUID *compartment_guid);

void langbaritembutton_cleanup(ITfLangBarItemButton* button);

HRESULT langbaritembutton_set_status(ITfLangBarItemButton* button, uint32_t status, bool set);

} // extern "C"
