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

void *compartment_new(void *punk, uint32_t tf_client_id, const GUID *guid);

HRESULT compartment_set_bool(void *compartment, bool flag);

HRESULT compartment_set_u32(void *compartment, uint32_t data);

HRESULT compartmenteventsink_advise(ITfCompartmentEventSink* sink,
                                    IUnknown* punk,
                                    const GUID *guid);

void *compartmenteventsink_new(CESCALLBACK callback, const void *pv);

HRESULT compartmenteventsink_unadvise(ITfCompartmentEventSink* sink);

} // extern "C"
