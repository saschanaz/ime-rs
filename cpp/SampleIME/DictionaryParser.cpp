// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "DictionaryParser.h"
#include "SampleIMEBaseStructure.h"
#include "RustStringRange.h"
#include "..\..\rust\dictionary_parser\dictionary_parser.h"

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "userenv.lib")

//---------------------------------------------------------------------
//
// ctor
//
//---------------------------------------------------------------------

CDictionaryParser::CDictionaryParser(LCID locale)
{
    _locale = locale;
}

//---------------------------------------------------------------------
//
// dtor
//
//---------------------------------------------------------------------

CDictionaryParser::~CDictionaryParser()
{
}

//---------------------------------------------------------------------
//
// ParseLine
//
// dwBufLen - in character count
//
//---------------------------------------------------------------------

std::optional<std::tuple<CRustStringRange, CRustStringRange>> CDictionaryParser::ParseLine(const CRustStringRange& input)
{
    void* keyword_raw;
    void* value_raw;

    if (!parse_line(input.GetInternal(), &keyword_raw, &value_raw))
    {
        return std::nullopt;
    }

    return std::make_tuple(CRustStringRange::from_void(keyword_raw), CRustStringRange::from_void(value_raw));
}

//---------------------------------------------------------------------
//
// GetOneLine
//
// dwBufLen - in character count
//
//---------------------------------------------------------------------

DWORD_PTR CDictionaryParser::GetOneLine(_In_z_ LPCSTR pwszBuffer, DWORD_PTR dwBufLen)
{
    DWORD_PTR dwIndexTrace = 0;     // in char

    if (FAILED(FindChar(u8'\r', pwszBuffer, dwBufLen, &dwIndexTrace)))
    {
        if (FAILED(FindChar(u8'\0', pwszBuffer, dwBufLen, &dwIndexTrace)))
        {
            return dwBufLen;
        }
    }

    return dwIndexTrace;
}
