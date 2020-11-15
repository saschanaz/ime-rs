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

CRustTableDictionaryEngine::~CRustTableDictionaryEngine() {
    if (engine) {
        tabledictionaryengine_free(engine);
    }
}

std::optional<CRustTableDictionaryEngine> CRustTableDictionaryEngine::Load(CRustStringRange path, bool sort) {
    void* engine = tabledictionaryengine_load(path.GetInternal(), sort);

    if (engine) {
        return CRustTableDictionaryEngine(engine);
    }

    return std::nullopt;
}

inline void arrays_to_array(void** keys, void** values, uintptr_t length, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList) {
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

void CRustTableDictionaryEngine::CollectWord(const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList) const {
    void* keys[MAX_BUFFER];
    void* values[MAX_BUFFER];
    uintptr_t length = tabledictionaryengine_collect_word(this->engine, keyCode.GetInternal(), keys, values, MAX_BUFFER);
    arrays_to_array(keys, values, length, pItemList);
}

//+---------------------------------------------------------------------------
//
// CollectWordForWildcard
//
//----------------------------------------------------------------------------


void CRustTableDictionaryEngine::CollectWordForWildcard(const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList) const {
    void* keys[MAX_BUFFER];
    void* values[MAX_BUFFER];
    uintptr_t length = tabledictionaryengine_collect_word_for_wildcard(this->engine, keyCode.GetInternal(), keys, values, MAX_BUFFER);
    arrays_to_array(keys, values, length, pItemList);
}

//+---------------------------------------------------------------------------
//
// CollectWordFromConvertedStringForWildcard
//
//----------------------------------------------------------------------------


void CRustTableDictionaryEngine::CollectWordFromConvertedStringForWildcard(const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList) const {
    void* keys[MAX_BUFFER];
    void* values[MAX_BUFFER];
    uintptr_t length = tabledictionaryengine_collect_word_from_converted_string_for_wildcard(this->engine, keyCode.GetInternal(), keys, values, MAX_BUFFER);
    arrays_to_array(keys, values, length, pItemList);
}

