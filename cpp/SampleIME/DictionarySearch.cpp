// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "DictionarySearch.h"
#include "SampleIMEBaseStructure.h"

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

BOOL CDictionarySearch::FindWorker(BOOL isTextSearch, _Out_ CDictionaryResult **ppdret, BOOL isWildcardSearch)
{
    DWORD_PTR dwTotalBufLen = GetBufferInWCharLength();        // in char
    if (dwTotalBufLen == 0)
    {
        return FALSE;
    }

    const WCHAR *pwch = GetBufferInWChar();
    DWORD_PTR indexTrace = 0;     // in char
    *ppdret = nullptr;
    BOOL isFound = FALSE;
    DWORD_PTR bufLenOneLine = 0;

TryAgain:
    bufLenOneLine = GetOneLine(&pwch[indexTrace], dwTotalBufLen);
    if (bufLenOneLine == 0)
    {
        goto FindNextLine;
    }
    else
    {
        CStringRangeSmart line;
        CStringRangeSmart keyword;

        line.SetClone(&pwch[indexTrace], bufLenOneLine);

        if (!ParseLine(line, &keyword))
        {
            return FALSE;    // error
        }

        if (!isTextSearch)
        {
            // Compare Dictionary key code and input key code
            if (!isWildcardSearch)
            {
                if (CStringRangeSmart::Compare(_locale, keyword, _searchKeyCode) != CSTR_EQUAL)
                {
                    goto FindNextLine;
                }
            }
            else
            {
                // Wildcard search
                if (!CStringRangeSmart::WildcardCompare(_locale, _searchKeyCode, keyword))
                {
                    goto FindNextLine;
                }
            }
        }
        else
        {
            // Compare Dictionary converted string and input string
            CStringRangeSmart tempString;
            if (!ParseLine(line, &keyword, &tempString))
            {
                return FALSE;
            }
            if (tempString.GetLength())
            {
                if (!isWildcardSearch)
                {
                    if (CStringRangeSmart::Compare(_locale, tempString, _searchKeyCode) != CSTR_EQUAL)
                    {
                        goto FindNextLine;
                    }
                }
                else
                {
                    // Wildcard search
                    if (!CStringRangeSmart::WildcardCompare(_locale, _searchKeyCode, tempString))
                    {
                        goto FindNextLine;
                    }
                }
            }
            else
            {
                goto FindNextLine;
            }
        }

        // Prepare return's CDictionaryResult
        *ppdret = new (std::nothrow) CDictionaryResult();
        if (!*ppdret)
        {
            return FALSE;
        }

        CStringRangeSmart valueString;
        if (!ParseLine(line, &keyword, &valueString))
        {
            if (*ppdret)
            {
                delete *ppdret;
                *ppdret = nullptr;
            }
            return FALSE;
        }

        (*ppdret)->_FindKeyCode = keyword;
        (*ppdret)->_SearchKeyCode = _searchKeyCode;

        (*ppdret)->_FindPhraseList.Append(valueString);

        // Seek to next line
        isFound = TRUE;
    }

FindNextLine:
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
    if (pwch[indexTrace] == L'\r' || pwch[indexTrace] == L'\n' || pwch[indexTrace] == L'\0')
    {
        bufLenOneLine = 1;
        goto FindNextLine;
    }

    if (isFound)
    {
        _charIndex += indexTrace;
        return TRUE;
    }

    goto TryAgain;
}
