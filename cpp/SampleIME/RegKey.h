// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

class CRegKey
{
public:
    CRegKey();
    ~CRegKey();

    LONG Create(_In_ HKEY hKeyPresent, _In_ LPCWSTR pwszKeyName,
        _In_reads_opt_(255) LPWSTR pwszClass = REG_NONE,
        DWORD dwOptions = REG_OPTION_NON_VOLATILE,
        REGSAM samDesired = KEY_READ | KEY_WRITE,
        _Inout_ LPSECURITY_ATTRIBUTES lpSecAttr = nullptr,
        _Out_opt_ LPDWORD lpdwDisposition = nullptr);

    LONG Open(_In_ HKEY hKeyParent, _In_ LPCWSTR pwszKeyName,
        REGSAM samDesired = KEY_READ | KEY_WRITE);

    LONG Close();

    LONG QueryStringValue(_In_opt_ LPCWSTR pwszValueName, _Out_writes_opt_(*pnChars) LPWSTR pwszValue, _Inout_ ULONG *pnChars);

private:
    HKEY  _keyHandle;
};
