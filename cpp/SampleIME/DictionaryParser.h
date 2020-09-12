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

    BOOL ParseLine(const CStringRangeSmart& input, _Out_ CStringRangeSmart* psrgKeyword, _Out_ CStringRangeSmart *psrgValue);

    // dwBufLen - in character count
    _Ret_maybenull_
    LPCWSTR GetToken(const CStringRangeSmart& input, _In_ const WCHAR chDelimiter, _Out_ CStringRangeSmart *psrgKeyWord);

protected:
    BOOL RemoveWhiteSpaceFromBegin(CStringRangeSmart& string);
    BOOL RemoveWhiteSpaceFromEnd(CStringRangeSmart& string);
    BOOL RemoveStringDelimiter(CStringRangeSmart& string);

    DWORD_PTR GetOneLine(_In_z_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen);

    LCID _locale;   // used for CompareString
};
