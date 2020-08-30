// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "Globals.h"

//////////////////////////////////////////////////////////////////////
//

class CDictionaryParser
{
public:
    CDictionaryParser(LCID locale);
    virtual ~CDictionaryParser();

    BOOL ParseLine(const CStringRangeSmart& input, _Out_ CStringRangeSmart* psrgKeyword, _Out_ CStringRangeSmart *psrgValue = nullptr);

    // dwBufLen - in character count
    _Ret_maybenull_
    LPCWSTR GetToken(_In_reads_(dwBufLen) LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _In_ const WCHAR chDelimiter, _Out_ CStringRange *srgKeyWord);

protected:
    BOOL RemoveWhiteSpaceFromBegin(_Inout_opt_ CStringRange *pString);
    BOOL RemoveWhiteSpaceFromEnd(_Inout_opt_ CStringRange *pString);
    BOOL RemoveStringDelimiter(_Inout_opt_ CStringRange *pString);

    DWORD_PTR GetOneLine(_In_z_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen);

    LCID _locale;   // used for CompareString
};
