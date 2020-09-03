// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "DictionaryParser.h"
#include "SampleIMEBaseStructure.h"
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

BOOL CDictionaryParser::ParseLine(const CStringRangeSmart& input, _Out_ CStringRangeSmart* psrgKeyword, _Out_ CStringRangeSmart *psrgValue)
{
    LPCWSTR pwszKeyWordDelimiter = nullptr;
    pwszKeyWordDelimiter = GetToken(input, Global::KeywordDelimiter, psrgKeyword);
    if (!(pwszKeyWordDelimiter))
    {
        return FALSE;    // End of file
    }

    CStringRangeSmart value = input.Substr(pwszKeyWordDelimiter - input.GetRaw() + 1);

    // Get value.
    if (psrgValue && value.GetLength())
    {
        RemoveWhiteSpaceFromBegin(value);
        RemoveWhiteSpaceFromEnd(value);
        RemoveStringDelimiter(value);
        *psrgValue = value;
    }

    return TRUE;
}

//---------------------------------------------------------------------
//
// GetToken
//
// dwBufLen - in character count
//
// return   - pointer of delimiter which specified chDelimiter
//
//---------------------------------------------------------------------
_Ret_maybenull_
LPCWSTR CDictionaryParser::GetToken(const CStringRangeSmart& input, _In_ const WCHAR, _Out_ CStringRangeSmart *psrgValue)
{
    *psrgValue = input;

    LPCWSTR result = (LPCWSTR)get_equalsign((uint16_t*)input.GetRaw(), (uintptr_t)input.GetLength());
    if (!result) {
        return nullptr;
    }

    LPCWSTR pwszStart = psrgValue->GetRaw();
    *psrgValue = psrgValue->Substr(0, result - pwszStart);

    RemoveWhiteSpaceFromBegin(*psrgValue);
    RemoveWhiteSpaceFromEnd(*psrgValue);
    RemoveStringDelimiter(*psrgValue);

    return result;
}

//---------------------------------------------------------------------
//
// RemoveWhiteSpaceFromBegin
// RemoveWhiteSpaceFromEnd
// RemoveStringDelimiter
//
//---------------------------------------------------------------------

BOOL CDictionaryParser::RemoveWhiteSpaceFromBegin(CStringRangeSmart& string)
{
    DWORD_PTR dwIndexTrace = 0;  // in char

    if (SkipWhiteSpace(_locale, string.GetRaw(), string.GetLength(), &dwIndexTrace) != S_OK)
    {
        return FALSE;
    }

    string = string.Substr(dwIndexTrace);
    return TRUE;
}

BOOL CDictionaryParser::RemoveWhiteSpaceFromEnd(CStringRangeSmart& string)
{
    DWORD_PTR dwTotalBufLen = string.GetLength();
    WCHAR lastChar = string.CharAt(dwTotalBufLen - 1);

    while (dwTotalBufLen && (IsSpace(_locale, lastChar) || lastChar == L'\r' || lastChar == L'\n'))
    {
        dwTotalBufLen--;
        lastChar = string.CharAt(dwTotalBufLen - 1);
    }

    string = string.Substr(0, dwTotalBufLen);
    return TRUE;
}

BOOL CDictionaryParser::RemoveStringDelimiter(CStringRangeSmart& string)
{
    if (string.GetLength() >= 2)
    {
        if ((string.CharAt(0) == Global::StringDelimiter) && (string.CharAt(string.GetLength()-1) == Global::StringDelimiter))
        {
            string = string.Substr(1, string.GetLength() - 1);
            return TRUE;
        }
    }

    return FALSE;
}

//---------------------------------------------------------------------
//
// GetOneLine
//
// dwBufLen - in character count
//
//---------------------------------------------------------------------

DWORD_PTR CDictionaryParser::GetOneLine(_In_z_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen)
{
    DWORD_PTR dwIndexTrace = 0;     // in char

    if (FAILED(FindChar(L'\r', pwszBuffer, dwBufLen, &dwIndexTrace)))
    {
        if (FAILED(FindChar(L'\0', pwszBuffer, dwBufLen, &dwIndexTrace)))
        {
            return dwBufLen;
        }
    }

    return dwIndexTrace;
}
