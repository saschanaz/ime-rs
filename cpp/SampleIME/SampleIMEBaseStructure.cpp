// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <codecvt>

#include "Globals.h"
#include "RustStringRange.h"

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "userenv.lib")

//---------------------------------------------------------------------
//
// CLSIDToString
//
//---------------------------------------------------------------------

const BYTE GuidSymbols[] = {
    3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15
};

const WCHAR HexDigits[] = L"0123456789ABCDEF";

BOOL CLSIDToString(REFGUID refGUID, _Out_writes_(39) WCHAR *pCLSIDString)
{
    WCHAR* pTemp = pCLSIDString;
    const BYTE* pBytes = (const BYTE *) &refGUID;

    DWORD j = 0;
    pTemp[j++] = L'{';
    for (int i = 0; i < sizeof(GuidSymbols) && j < (CLSID_STRLEN - 2); i++)
    {
        if (GuidSymbols[i] == '-')
        {
            pTemp[j++] = L'-';
        }
        else
        {
            pTemp[j++] = HexDigits[ (pBytes[GuidSymbols[i]] & 0xF0) >> 4 ];
            pTemp[j++] = HexDigits[ (pBytes[GuidSymbols[i]] & 0x0F) ];
        }
    }

    pTemp[j++] = L'}';
    pTemp[j] = L'\0';

    return TRUE;
}

//---------------------------------------------------------------------
//
// SkipWhiteSpace
//
// dwBufLen - in character count
//
//---------------------------------------------------------------------

HRESULT SkipWhiteSpace(LCID locale, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex)
{
    DWORD_PTR index = 0;

    *pdwIndex = 0;
    while (*pwszBuffer && IsSpace(locale, *pwszBuffer) && dwBufLen)
    {
        dwBufLen--;
        pwszBuffer++;
        index++;
    }

    if (*pwszBuffer && dwBufLen)
    {
        *pdwIndex = index;
        return S_OK;
    }

    return E_FAIL;
}

//---------------------------------------------------------------------
//
// FindChar
//
// dwBufLen - in character count
//
//---------------------------------------------------------------------

HRESULT FindChar(char wch, _In_ LPCSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex)
{
    DWORD_PTR index = 0;

    *pdwIndex = 0;
    while (*pwszBuffer && (*pwszBuffer != wch) && dwBufLen)
    {
        dwBufLen--;
        pwszBuffer++;
        index++;
    }

    if (*pwszBuffer && dwBufLen)
    {
        *pdwIndex = index;
        return S_OK;
    }

    return E_FAIL;
}

//---------------------------------------------------------------------
//
// IsSpace
//
//---------------------------------------------------------------------

BOOL IsSpace(LCID locale, WCHAR wch)
{
    WORD wCharType = 0;

    GetStringTypeEx(locale, CT_CTYPE1, &wch, 1, &wCharType);
    return (wCharType & C1_SPACE);
}

const DWORD_PTR CStringRangeUtf16::GetLength() const
{
    return _stringBufLen;
}

const WCHAR *CStringRangeUtf16::GetRaw() const
{
    return _pStringBuf.get();
}

void CStringRangeUtf16::SetClone(const WCHAR *pwch, DWORD_PTR dwLength)
{
    _stringBufLen = dwLength;
    _pStringBuf = std::shared_ptr<WCHAR>(Clone(pwch, _stringBufLen));
}

CStringRangeUtf16::CStringRangeUtf16(WCHAR wch)
{
    _stringBufLen = 1;
    _pStringBuf = std::make_shared<WCHAR>(wch);
}

CStringRangeUtf16::CStringRangeUtf16(const CRustStringRange& rsr) {
    // The conversion to UTF16 is in C++ on purpose to allow easier memory management
    char* firstChar = (char*)rsr.GetRawUtf8();
    char* afterLastChar = firstChar + rsr.GetLengthUtf8();
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> conversion;
    std::u16string strU16 = conversion.from_bytes(firstChar, afterLastChar);

    SetClone((WCHAR*)strU16.c_str(), strU16.length());
}

CStringRangeUtf16::operator CRustStringRange() const {
    return CRustStringRange(GetRaw(), GetLength());
};

WCHAR* CStringRangeUtf16::Clone(const WCHAR* pwch, DWORD_PTR dwLength)
{
    if (!dwLength) {
        return nullptr;
    }
    WCHAR* pwchString = new (std::nothrow) WCHAR[ dwLength ];
    if (!pwchString)
    {
        return nullptr;
    }
    memcpy((void*)pwchString, pwch, dwLength * sizeof(WCHAR));
    return pwchString;
}

BOOL CCandidateRange::IsRange(UINT vKey)
{
    int index = GetIndex(vKey);
    return index != -1;
}

int CCandidateRange::GetIndex(UINT vKey)
{
    DWORD value = ((VK_NUMPAD0 <= vKey) && (vKey <= VK_NUMPAD9)) ? (vKey - VK_NUMPAD0) : (vKey - L'0');

    if (value == 0) {
        return 9;
    }
    if (value > 0 && value < 10) {
        return value - 1;
    }
    return -1;
}

CPunctuationPair::CPunctuationPair()
{
    _punctuation._Code = 0;
    _punctuation._Punctuation = 0;
    _pairPunctuation = 0;
    _isPairToggle = FALSE;
}

CPunctuationPair::CPunctuationPair(WCHAR code, WCHAR punctuation, WCHAR pair)
{
    _punctuation._Code = code;
    _punctuation._Punctuation = punctuation;
    _pairPunctuation = pair;
    _isPairToggle = FALSE;
}

CPunctuationNestPair::CPunctuationNestPair()
{
    _punctuation_begin._Code = 0;
    _punctuation_begin._Punctuation = 0;
    _pairPunctuation_begin = 0;

    _punctuation_end._Code = 0;
    _punctuation_end._Punctuation = 0;
    _pairPunctuation_end = 0;

    _nestCount = 0;
}

CPunctuationNestPair::CPunctuationNestPair(WCHAR codeBegin, WCHAR punctuationBegin, WCHAR pairBegin,
    WCHAR codeEnd,   WCHAR punctuationEnd,   WCHAR pairEnd)
{
	pairEnd;punctuationEnd;
    _punctuation_begin._Code = codeBegin;
    _punctuation_begin._Punctuation = punctuationBegin;
    _pairPunctuation_begin = pairBegin;

    _punctuation_end._Code = codeEnd;
    _punctuation_end._Punctuation = punctuationBegin;
    _pairPunctuation_end = pairBegin;

    _nestCount  = 0;
}
