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
    bool weak;

    CRustTableDictionaryEngine(void* engine, bool weak) {
        this->engine = engine;
        this->weak = weak;
    }

public:
    ~CRustTableDictionaryEngine();

    static CRustTableDictionaryEngine WeakRef(void* engine) {
        return CRustTableDictionaryEngine(engine, true);
    }

    CRustTableDictionaryEngine(CRustTableDictionaryEngine&& that) noexcept {
        engine = that.engine;
        weak = that.weak;
        that.engine = nullptr;
        that.weak = false;
    }

    CRustTableDictionaryEngine& operator=(CRustTableDictionaryEngine en) {
        std::swap(engine, en.engine);
        std::swap(weak, en.weak);
        return *this;
    }

    static std::optional<CRustTableDictionaryEngine> Load(CRustStringRange path, bool sort);

    void CollectWord(const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList) const;

    void CollectWordForWildcard(const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList) const;

    void CollectWordFromConvertedStringForWildcard(const CRustStringRange& string, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList) const;
};
