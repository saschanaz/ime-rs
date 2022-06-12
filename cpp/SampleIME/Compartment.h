// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <cstdint>

class CCompartment
{
public:
    CCompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, _In_ REFGUID guidCompartment);
    ~CCompartment();

    HRESULT _GetCompartment(_Outptr_ ITfCompartment **ppCompartment);
    HRESULT _GetCompartmentBOOL(bool& flag);
    HRESULT _SetCompartmentBOOL(bool flag);
    HRESULT _GetCompartmentU32(uint32_t &dw);
    HRESULT _SetCompartmentU32(uint32_t dw);
    HRESULT _ClearCompartment();

    void _GetGUID(GUID* pguid) const;

private:
    void* compartment;
};
