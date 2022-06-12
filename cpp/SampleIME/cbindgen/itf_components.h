#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <new>

using CESCALLBACK = HRESULT(*)(const void *pv, const GUID *guid);

extern "C" {

HRESULT compartment_clear(void *compartment);

void compartment_free(void *compartment);

HRESULT compartment_get_bool(void *compartment, bool *data);

HRESULT compartment_get_u32(void *compartment, uint32_t *data);

void compartment_guid(void *compartment, GUID *guid);

void *compartment_new(ITfThreadMgr* thread_mgr, uint32_t tf_client_id, const GUID *guid);

HRESULT compartment_set_bool(void *compartment, bool flag);

HRESULT compartment_set_u32(void *compartment, uint32_t data);

HRESULT compartmenteventsink_advise(ITfCompartmentEventSink* sink,
                                    IUnknown* punk,
                                    const GUID *guid);

void *compartmenteventsink_new(CESCALLBACK callback, const void *pv);

HRESULT compartmenteventsink_unadvise(ITfCompartmentEventSink* sink);

void langbaritembutton_cleanup(ITfLangBarItemButton* button);

HRESULT langbaritembutton_init(ITfLangBarItemButton* button,
                               ITfThreadMgr* thread_mgr,
                               uint32_t tf_client_id,
                               const GUID *compartment_guid);

void *langbaritembutton_new(const GUID *item_guid,
                            void *description,
                            void *tooltip,
                            uint32_t on_icon_index,
                            uint32_t off_icon_index);

HRESULT langbaritembutton_set_status(ITfLangBarItemButton* button, uint32_t status, bool set);

} // extern "C"
