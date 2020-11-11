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
#include "..\..\rust\dictionary_parser\dictionary_parser.h"

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CDictionarySearch::CDictionarySearch(LCID locale, _In_ CFile *pFile, const CRustStringRange& searchKeyCode) : CDictionaryParser(locale), _searchKeyCode(searchKeyCode)
{
    _pFile = pFile;
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

static bool StringCompare(const CRustStringRange& x, const CRustStringRange& y, LCID locale, bool isWildcardSearch)
{
    if (isWildcardSearch)
    {
        return x.CompareWithWildCard(y);
    }
    return x == y;
}

BOOL CDictionarySearch::FindWorker(BOOL isTextSearch, _Out_ CDictionaryResult **ppdret, BOOL isWildcardSearch)
{
    CRustStringRange range(GetBufferInChar(), GetBufferInCharLength());

    void* key_raw;
    void* value_raw;
    uintptr_t offset = find_worker(range.GetInternal(), _searchKeyCode.GetInternal(), isTextSearch, isWildcardSearch, &key_raw, &value_raw);
    _charIndex += offset;

    if (key_raw) {
        // Prepare return's CDictionaryResult
        *ppdret = new (std::nothrow) CDictionaryResult(_searchKeyCode, CRustStringRange::from_void(key_raw), CRustStringRange::from_void(value_raw));
        if (!*ppdret)
        {
            return false;
        }

        return true;
    }

    return false;
}
