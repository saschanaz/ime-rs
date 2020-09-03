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
        CStringRange valueRaw = value.ToRaw();

        RemoveWhiteSpaceFromBegin(&valueRaw);
        RemoveWhiteSpaceFromEnd(&valueRaw);
        RemoveStringDelimiter(&valueRaw);
        *psrgValue = valueRaw;
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

    CStringRange psrgValueRaw = psrgValue->ToRaw();
    RemoveWhiteSpaceFromBegin(&psrgValueRaw);
    RemoveWhiteSpaceFromEnd(&psrgValueRaw);
    RemoveStringDelimiter(&psrgValueRaw);
    *psrgValue = psrgValueRaw;

    return result;
}

//---------------------------------------------------------------------
//
// RemoveWhiteSpaceFromBegin
// RemoveWhiteSpaceFromEnd
// RemoveStringDelimiter
//
//---------------------------------------------------------------------

BOOL CDictionaryParser::RemoveWhiteSpaceFromBegin(_Inout_opt_ CStringRange *pString)
{
    DWORD_PTR dwIndexTrace = 0;  // in char

    if (pString == nullptr)
    {
        return FALSE;
    }

    if (SkipWhiteSpace(_locale, pString->Get(), pString->GetLength(), &dwIndexTrace) != S_OK)
    {
        return FALSE;
    }

    pString->Set(pString->Get() + dwIndexTrace, pString->GetLength() - dwIndexTrace);
    return TRUE;
}

BOOL CDictionaryParser::RemoveWhiteSpaceFromEnd(_Inout_opt_ CStringRange *pString)
{
    if (pString == nullptr)
    {
        return FALSE;
    }

    DWORD_PTR dwTotalBufLen = pString->GetLength();
    LPCWSTR pwszEnd = pString->Get() + dwTotalBufLen - 1;

    while (dwTotalBufLen && (IsSpace(_locale, *pwszEnd) || *pwszEnd == L'\r' || *pwszEnd == L'\n'))
    {
        pwszEnd--;
        dwTotalBufLen--;
    }

    pString->Set(pString->Get(), dwTotalBufLen);
    return TRUE;
}

BOOL CDictionaryParser::RemoveStringDelimiter(_Inout_opt_ CStringRange *pString)
{
    if (pString == nullptr)
    {
        return FALSE;
    }

    if (pString->GetLength() >= 2)
    {
        if ((*pString->Get() == Global::StringDelimiter) && (*(pString->Get()+pString->GetLength()-1) == Global::StringDelimiter))
        {
            pString->Set(pString->Get()+1, pString->GetLength()-2);
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
