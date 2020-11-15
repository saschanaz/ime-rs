// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <optional>

#include "SampleIMEBaseStructure.h"
#include "RustStringRange.h"

class CRustTableDictionaryEngine {
    void* engine;

    CRustTableDictionaryEngine(void* engine) {
        this->engine = engine;
    }

public:
    ~CRustTableDictionaryEngine();

    CRustTableDictionaryEngine(CRustTableDictionaryEngine&& that) noexcept {
        engine = that.engine;
        that.engine = nullptr;
    }

    CRustTableDictionaryEngine& operator=(CRustTableDictionaryEngine en) {
        std::swap(engine, en.engine);
        return *this;
    }

    static std::optional<CRustTableDictionaryEngine> Load(CRustStringRange path, bool sort);

    void CollectWord(const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList) const;

    void CollectWordForWildcard(const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList) const;

    void CollectWordFromConvertedStringForWildcard(const CRustStringRange& string, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList) const;
};
