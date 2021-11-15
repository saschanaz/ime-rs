// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Compartment.h"
#include "Globals.h"
#include "cbindgen/itf_components.h"

//////////////////////////////////////////////////////////////////////
//
// CCompartment
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------------

CCompartment::CCompartment(_In_ IUnknown* punk, TfClientId tfClientId, _In_ REFGUID guidCompartment)
{
    compartment = compartment_new(punk, tfClientId, &guidCompartment);
}

//+---------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------

CCompartment::~CCompartment()
{
    compartment_free(compartment);
}

//+---------------------------------------------------------------------------
// _GetCompartmentBOOL
//----------------------------------------------------------------------------

HRESULT CCompartment::_GetCompartmentBOOL(bool& flag)
{
    return compartment_get_bool(compartment, &flag);
}

//+---------------------------------------------------------------------------
// _SetCompartmentBOOL
//----------------------------------------------------------------------------

HRESULT CCompartment::_SetCompartmentBOOL(bool flag)
{
    return _SetCompartmentU32(flag);
}

//+---------------------------------------------------------------------------
// _GetCompartmentU32
//----------------------------------------------------------------------------

HRESULT CCompartment::_GetCompartmentU32(uint32_t& dw)
{
    return compartment_get_u32(compartment, &dw);
}

//+---------------------------------------------------------------------------
// _SetCompartmentU32
//----------------------------------------------------------------------------

HRESULT CCompartment::_SetCompartmentU32(uint32_t dw)
{
    return compartment_set_u32(compartment, dw);
}

//+---------------------------------------------------------------------------
//
// _ClearCompartment
//
//----------------------------------------------------------------------------

HRESULT CCompartment::_ClearCompartment()
{
    return compartment_clear(compartment);
}

void CCompartment::_GetGUID(GUID* pguid) const {
    compartment_guid(compartment, pguid);
}

//////////////////////////////////////////////////////////////////////
//
// CCompartmentEventSink
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------------

CCompartmentEventSink::CCompartmentEventSink(_In_ CESCALLBACK pfnCallback, _In_ void *pv)
{
    _pfnCallback = pfnCallback;
    _pv = pv;
    _refCount = 1;
}

//+---------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------

CCompartmentEventSink::~CCompartmentEventSink()
{
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CCompartmentEventSink::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
{
    if (ppvObj == nullptr)
        return E_INVALIDARG;

    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfCompartmentEventSink))
    {
        *ppvObj = (CCompartmentEventSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


//+---------------------------------------------------------------------------
//
// AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCompartmentEventSink::AddRef()
{
    return ++_refCount;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCompartmentEventSink::Release()
{
    LONG cr = --_refCount;

    assert(_refCount >= 0);

    if (_refCount == 0)
    {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// OnChange
//
//----------------------------------------------------------------------------

STDAPI CCompartmentEventSink::OnChange(_In_ REFGUID guidCompartment)
{
    return _pfnCallback(_pv, guidCompartment);
}

//+---------------------------------------------------------------------------
//
// _Advise
//
//----------------------------------------------------------------------------

HRESULT CCompartmentEventSink::_Advise(_In_ IUnknown *punk, _In_ REFGUID guidCompartment)
{
    HRESULT hr = S_OK;
    ITfCompartmentMgr* pCompartmentMgr = nullptr;
    ITfSource* pSource = nullptr;

    hr = punk->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pCompartmentMgr->GetCompartment(guidCompartment, &_pCompartment);
    if (SUCCEEDED(hr))
    {
        hr = _pCompartment->QueryInterface(IID_ITfSource, (void **)&pSource);
        if (SUCCEEDED(hr))
        {
            hr = pSource->AdviseSink(IID_ITfCompartmentEventSink, this, &_dwCookie);
            pSource->Release();
        }
    }

    pCompartmentMgr->Release();

    return hr;
}

//+---------------------------------------------------------------------------
//
// _Unadvise
//
//----------------------------------------------------------------------------

HRESULT CCompartmentEventSink::_Unadvise()
{
    HRESULT hr = S_OK;
    ITfSource* pSource = nullptr;

    hr = _pCompartment->QueryInterface(IID_ITfSource, (void **)&pSource);
    if (SUCCEEDED(hr))
    {
        hr = pSource->UnadviseSink(_dwCookie);
        pSource->Release();
    }

    _pCompartment->Release();
    _pCompartment = nullptr;
    _dwCookie = 0;

    return hr;
}
