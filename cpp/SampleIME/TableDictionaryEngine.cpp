// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "TableDictionaryEngine.h"
#include "../../rust/dictionary_parser/dictionary_parser.h"

static const uintptr_t MAX_BUFFER = 512;

inline void Collect(const CRustStringRange& range, const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList, bool isTextSearch, bool isWildcardSearch) {
    void* keys[MAX_BUFFER];
    void* values[MAX_BUFFER];
    uintptr_t length = find_all(range.GetInternal(), keyCode.GetInternal(), isTextSearch, isWildcardSearch, keys, values, MAX_BUFFER);

    for (uintptr_t i = 0; i < length; i++) {
        CRustStringRange key = CRustStringRange::from_void(keys[i]);
        CRustStringRange value = CRustStringRange::from_void(values[i]);
        CCandidateListItem listItem(value, key);
        pItemList->Append(listItem);
    }
}

//+---------------------------------------------------------------------------
//
// CollectWord
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::CollectWord(const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CRustStringRange range(_pDictionaryFile->GetReadBufferPointer(), _pDictionaryFile->GetFileSize());
    Collect(range, keyCode, pItemList, false, false);
}

//+---------------------------------------------------------------------------
//
// CollectWordForWildcard
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::CollectWordForWildcard(const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CRustStringRange range(_pDictionaryFile->GetReadBufferPointer(), _pDictionaryFile->GetFileSize());
    Collect(range, keyCode, pItemList, false, true);
}

//+---------------------------------------------------------------------------
//
// CollectWordFromConvertedStringForWildcard
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::CollectWordFromConvertedStringForWildcard(const CRustStringRange& string, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CRustStringRange range(_pDictionaryFile->GetReadBufferPointer(), _pDictionaryFile->GetFileSize());
    Collect(range, string, pItemList, true, true);
}

