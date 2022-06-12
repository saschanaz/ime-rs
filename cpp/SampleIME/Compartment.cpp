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

CCompartment::CCompartment(_In_ ITfThreadMgr* pThreadMgr, TfClientId tfClientId, _In_ REFGUID guidCompartment)
{
    pThreadMgr->AddRef();
    compartment = compartment_new(pThreadMgr, tfClientId, &guidCompartment);
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
    return compartment_set_bool(compartment, flag);
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
