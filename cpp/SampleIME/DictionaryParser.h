// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include <optional>

#include "Globals.h"

//////////////////////////////////////////////////////////////////////
//

class CDictionaryParser
{
public:
    CDictionaryParser(LCID locale);
    virtual ~CDictionaryParser();

    std::optional<std::tuple<CRustStringRange, CRustStringRange>> ParseLine(const CRustStringRange& input);

protected:
    DWORD_PTR GetOneLine(_In_z_ LPCSTR pwszBuffer, DWORD_PTR dwBufLen);

    LCID _locale;   // used for CompareString
};
