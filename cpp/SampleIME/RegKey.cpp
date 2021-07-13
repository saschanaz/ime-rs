// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "RegKey.h"

//---------------------------------------------------------------------
//
// ctor
//
//---------------------------------------------------------------------

CRegKey::CRegKey()
{
    _keyHandle = nullptr;
}

//---------------------------------------------------------------------
//
// dtor
//
//---------------------------------------------------------------------

CRegKey::~CRegKey()
{
    Close();
}

//---------------------------------------------------------------------
//
// Create
//
//---------------------------------------------------------------------

LONG CRegKey::Create(_In_ HKEY hKeyPresent, _In_ LPCWSTR pwszKeyName, _In_reads_opt_(255) LPWSTR pwszClass, DWORD dwOptions, REGSAM samDesired, _Inout_ LPSECURITY_ATTRIBUTES lpSecAttr, _Out_opt_ LPDWORD lpdwDisposition)
{
    DWORD disposition = 0;
    HKEY keyHandle = nullptr;

    LONG res = RegCreateKeyEx(hKeyPresent, pwszKeyName, 0,
        pwszClass, dwOptions, samDesired, lpSecAttr, &keyHandle, &disposition);

    if (lpdwDisposition != nullptr)
    {
        *lpdwDisposition = disposition;
    }

    if (res == ERROR_SUCCESS)
    {
        Close();
        _keyHandle = keyHandle;
    }

    return res;
}

//---------------------------------------------------------------------
//
// Open
//
//---------------------------------------------------------------------

LONG CRegKey::Open(_In_ HKEY hKeyParent, _In_ LPCWSTR pwszKeyName, REGSAM samDesired)
{
    HKEY keyHandle = nullptr;

    LONG res = RegOpenKeyEx(hKeyParent, pwszKeyName, 0, samDesired, &keyHandle);
    if (res == ERROR_SUCCESS)
    {
        Close();
        _keyHandle = keyHandle;
    }
    return res;
}

//---------------------------------------------------------------------
//
// Close
//
//---------------------------------------------------------------------

LONG CRegKey::Close()
{
    LONG res = ERROR_SUCCESS;
    if (_keyHandle)
    {
        res = RegCloseKey(_keyHandle);
        _keyHandle = nullptr;
    }
    return res;
}

//---------------------------------------------------------------------
//
// QueryStingValue
// SetStringValue
//
//---------------------------------------------------------------------

LONG CRegKey::QueryStringValue(_In_opt_ LPCWSTR pwszValueName, _Out_writes_opt_(*pnChars) LPWSTR pwszValue, _Inout_ ULONG *pnChars)
{
    LONG res = 0;
    DWORD dataType = REG_NONE;
    ULONG pwszValueSize = 0;

    if (pnChars == nullptr)
    {
        return E_INVALIDARG;
    }

    pwszValueSize = (*pnChars)*sizeof(WCHAR);
    *pnChars = 0;

    res = RegQueryValueEx(_keyHandle, pwszValueName, NULL, &dataType, (LPBYTE)pwszValue, &pwszValueSize);
    if (res != ERROR_SUCCESS)
    {
        return res;
    }
    if ((dataType != REG_SZ) && (dataType != REG_EXPAND_SZ))
    {
        return ERROR_INVALID_DATA;
    }

    *pnChars = pwszValueSize / sizeof(WCHAR);

    return ERROR_SUCCESS;
}
