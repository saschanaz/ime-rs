// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "DictionarySearch.h"
#include "SampleIMEBaseStructure.h"
#include "RustStringRange.h"

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CDictionarySearch::CDictionarySearch(LCID locale, _In_ CFile *pFile, const CStringRangeSmart& searchKeyCode) : CDictionaryParser(locale)
{
    _pFile = pFile;
    _searchKeyCode = searchKeyCode;
    _charIndex = 0;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CDictionarySearch::~CDictionarySearch()
{
}

//+---------------------------------------------------------------------------
//
// FindPhrase
//
//----------------------------------------------------------------------------

BOOL CDictionarySearch::FindPhrase(_Out_ CDictionaryResult **ppdret)
{
    return FindWorker(FALSE, ppdret, FALSE); // NO WILDCARD
}

//+---------------------------------------------------------------------------
//
// FindPhraseForWildcard
//
//----------------------------------------------------------------------------

BOOL CDictionarySearch::FindPhraseForWildcard(_Out_ CDictionaryResult **ppdret)
{
    return FindWorker(FALSE, ppdret, TRUE); // Wildcard
}

//+---------------------------------------------------------------------------
//
// FindConvertedStringForWildcard
//
//----------------------------------------------------------------------------

BOOL CDictionarySearch::FindConvertedStringForWildcard(CDictionaryResult **ppdret)
{
    return FindWorker(TRUE, ppdret, TRUE); // Wildcard
}

//+---------------------------------------------------------------------------
//
// FindWorker
//
//----------------------------------------------------------------------------

static bool StringCompare(const CStringRangeSmart& x, const CStringRangeSmart& y, LCID locale, bool isWildcardSearch)
{
    if (isWildcardSearch)
    {
        return CStringRangeSmart::WildcardCompare(locale, x, y);
    }
    return CStringRangeSmart::Compare(locale, x, y) == CSTR_EQUAL;
}

BOOL CDictionarySearch::FindWorker(BOOL isTextSearch, _Out_ CDictionaryResult **ppdret, BOOL isWildcardSearch)
{
    DWORD_PTR dwTotalBufLen = GetBufferInCharLength();        // in char
    if (dwTotalBufLen == 0)
    {
        return FALSE;
    }

    const CHAR *pch = GetBufferInChar();
    DWORD_PTR indexTrace = 0;     // in char
    *ppdret = nullptr;
    BOOL isFound = FALSE;
    DWORD_PTR bufLenOneLine = 0;

    while (true)
    {
        bufLenOneLine = GetOneLine(&pch[indexTrace], dwTotalBufLen);
        if (bufLenOneLine)
        {
            CRustStringRange line(&pch[indexTrace], bufLenOneLine);
            CStringRangeSmart keyword;
            CStringRangeSmart value;

            if (!ParseLine(line, &keyword, &value))
            {
                return FALSE;    // error
            }

            const CStringRangeSmart& target = isTextSearch ? value : keyword;
            if (target.GetLength() && StringCompare(_searchKeyCode, target, _locale, isWildcardSearch))
            {
                // Prepare return's CDictionaryResult
                *ppdret = new (std::nothrow) CDictionaryResult();
                if (!*ppdret)
                {
                    return FALSE;
                }

                (*ppdret)->_FindKeyCode = keyword;
                (*ppdret)->_SearchKeyCode = _searchKeyCode;
                (*ppdret)->_FoundPhrase = value;

                // Seek to next line
                isFound = TRUE;
            }
        }

        while (true)
        {
            dwTotalBufLen -= bufLenOneLine;
            if (dwTotalBufLen == 0)
            {
                indexTrace += bufLenOneLine;
                _charIndex += indexTrace;

                if (!isFound && *ppdret)
                {
                    delete *ppdret;
                    *ppdret = nullptr;
                }
                return (isFound ? TRUE : FALSE);        // End of file
            }

            indexTrace += bufLenOneLine;
            if (pch[indexTrace] == u8'\r' || pch[indexTrace] == u8'\n' || pch[indexTrace] == u8'\0')
            {
                bufLenOneLine = 1;
            }
            else
            {
                break;
            }
        }

        if (isFound)
        {
            _charIndex += indexTrace;
            return TRUE;
        }
    }
}
