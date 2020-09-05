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

    BOOL ParseLine(const CRustStringRange& input, _Out_ CStringRangeSmart* psrgKeyword, _Out_ CStringRangeSmart *psrgValue);

protected:
    DWORD_PTR GetOneLine(_In_z_ LPCSTR pwszBuffer, DWORD_PTR dwBufLen);

    LCID _locale;   // used for CompareString
};
