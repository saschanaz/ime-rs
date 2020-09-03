// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Globals.h"
#include "..\..\rust\dictionary_parser\dictionary_parser.h"

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

HRESULT FindChar(WCHAR wch, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex)
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

CStringRange::CStringRange()
{
    _stringBufLen = 0;
    _pStringBuf = nullptr;
}

CStringRange::~CStringRange()
{
}

const WCHAR *CStringRange::Get() const
{
    return _pStringBuf;
}

const WCHAR *CStringRange::GetRaw() const
{
    return Get();
}

const DWORD_PTR CStringRangeBase::GetLength() const
{
    return _stringBufLen;
}

void CStringRange::Clear()
{
    _stringBufLen = 0;
    _pStringBuf = nullptr;
}

void CStringRange::Set(const WCHAR *pwch, DWORD_PTR dwLength)
{
    _stringBufLen = dwLength;
    _pStringBuf = pwch;
}

void CStringRange::Set(const CStringRange &sr)
{
    *this = sr;
}

CStringRange& CStringRange::operator =(const CStringRange& sr)
{
    _stringBufLen = sr._stringBufLen;
    _pStringBuf = sr._pStringBuf;
    return *this;
}

const WCHAR *CStringRangeSmart::GetRaw() const
{
    return _pStringBuf.get() + _startOffset;
}

WCHAR CStringRangeSmart::CharAt(DWORD_PTR index) const
{
    return *(GetRaw() + index);
}

void CStringRangeSmart::Clear()
{
    _stringBufLen = 0;
    _pStringBuf.reset();
    _startOffset = 0;
}

void CStringRangeSmart::SetClone(const WCHAR *pwch, DWORD_PTR dwLength)
{
    _stringBufLen = dwLength;
    _pStringBuf = std::shared_ptr<WCHAR>(Clone(pwch, _stringBufLen));
    _startOffset = 0;
}

void CStringRangeSmart::Set(const std::shared_ptr<const WCHAR> pwch, DWORD_PTR dwLength)
{
    _stringBufLen = dwLength;
    _pStringBuf = pwch;
    _startOffset = 0;
}

void CStringRangeSmart::Set(WCHAR wch)
{
    _stringBufLen = 1;
    _pStringBuf = std::make_shared<WCHAR>(wch);
    _startOffset = 0;
}

void CStringRangeSmart::Set(const CStringRange &sr)
{
    *this = sr;
}

void CStringRangeSmart::Set(const CStringRangeSmart &sr)
{
    *this = sr;
}

CStringRangeSmart& CStringRangeSmart::operator =(const CStringRange& sr)
{
    SetClone(sr.GetRaw(), sr.GetLength());
    return *this;
}

CStringRangeSmart& CStringRangeSmart::operator =(const CStringRangeSmart& sr)
{
    _stringBufLen = sr.GetLength();
    _pStringBuf = sr._pStringBuf;
    _startOffset = sr._startOffset;
    return *this;
}

int CStringRangeBase::Compare(LCID locale, const CStringRangeBase* pString1, const CStringRangeBase* pString2)
{
    return CompareString(locale,
        NORM_IGNORECASE,
        pString1->GetRaw(),
        (DWORD)pString1->GetLength(),
        pString2->GetRaw(),
        (DWORD)pString2->GetLength());
}

BOOL CStringRangeBase::WildcardCompare(LCID, const CStringRangeBase* stringWithWildcard, const CStringRangeBase* targetString)
{
    // This is expectedly slower than the previous C++ code
    // as the function now allocates a parsed string object every time
    // This should be faster when porting the string processing completes.
    return compare_with_wildcard((uint16_t*)stringWithWildcard->GetRaw(), stringWithWildcard->GetLength(), (uint16_t*)targetString->GetRaw(), targetString->GetLength());
}

CStringRangeSmart CStringRangeSmart::Substr(DWORD_PTR start) const {
    return Substr(start, GetLength());
}

CStringRangeSmart CStringRangeSmart::Substr(DWORD_PTR start, DWORD_PTR end) const {
    assert(start >= 0 && start <= end);
    assert(end <= GetLength());

    CStringRangeSmart range = *this;
    range._startOffset += start;
    range._stringBufLen = end - start;
    return range;
}

CStringRangeSmart CStringRangeSmart::Concat(const CStringRangeSmart& postfix) const {
    DWORD_PTR resultLength = GetLength() + postfix.GetLength();
    auto pwch = std::shared_ptr<WCHAR>(Clone(GetRaw(), resultLength));
    memcpy((void*)(pwch.get() + GetLength()), postfix.GetRaw(), postfix.GetLength() * sizeof(WCHAR));
    CStringRangeSmart range;
    range.Set(pwch, resultLength);
    return range;
}

CStringRange CStringRangeSmart::ToRaw() const
{
    CStringRange range;
    range.Set(GetRaw(), GetLength());
    return range;
}


WCHAR* CStringRangeSmart::Clone(const WCHAR* pwch, DWORD_PTR dwLength)
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

CStringRangeSmart operator""_sr(const wchar_t* aStr, std::size_t aLen) {
  CStringRangeSmart range;
  range.SetClone(aStr, aLen);
  return range;
}

CCandidateRange::CCandidateRange(void)
{
}


CCandidateRange::~CCandidateRange(void)
{
}


BOOL CCandidateRange::IsRange(UINT vKey)
{
    DWORD value = vKey - L'0';

    for (const auto& item : _CandidateListIndexRange)
    {
        if (value == item)
        {
            return TRUE;
        }
        else if ((VK_NUMPAD0 <= vKey) && (vKey <= VK_NUMPAD9))
        {
            if ((vKey-VK_NUMPAD0) == item)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

int CCandidateRange::GetIndex(UINT vKey)
{
    DWORD value = vKey - L'0';

    for (UINT i = 0; i < _CandidateListIndexRange.Count(); i++)
    {
        if (value == *_CandidateListIndexRange.GetAt(i))
        {
            return i;
        }
        else if ((VK_NUMPAD0 <= vKey) && (vKey <= VK_NUMPAD9))
        {
            if ((vKey-VK_NUMPAD0) == *_CandidateListIndexRange.GetAt(i))
            {
                return i;
            }
        }
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
