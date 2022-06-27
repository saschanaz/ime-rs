// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include <optional>
#include <tuple>

#include "sal.h"
#include "KeyHandlerEditSession.h"
#include "SampleIMEBaseStructure.h"
#include "Compartment.h"
#include "Define.h"
#include "RustStringRange.h"

class CCompositionProcessorEngine {
    void* engine;

public:
    // TODO: Ultimately split these fields as tuple<langid, profile, engine>,
    // since these fields are not used within this class.
    // For now let's keep them as fields to reduce code change.
    const LANGID langid;
    const GUID guidProfile;

    CCompositionProcessorEngine(LANGID langid, REFGUID guidLanguageProfile, ITfThreadMgr* threadMgr, TfClientId clientId);
    ~CCompositionProcessorEngine();

    void* GetRaw() { return engine; }

    bool SetupLanguageProfile(ITfThreadMgr *threadMgr, TfClientId clientId);

    bool AddVirtualKey(char16_t wch);
    void PopVirtualKey();
    void PurgeVirtualKey();
    bool HasVirtualKey();
    CRustStringRange KeystrokeBufferGetReadingString();
    bool KeystrokeBufferIncludesWildcard();

    void GetCandidateList(CSampleImeArray<CCandidateListItem> *pCandidateList, bool isIncrementalWordSearch, bool isWildcardSearch);
    void GetCandidateStringInConverted(const CRustStringRange& searchString, CSampleImeArray<CCandidateListItem> *pCandidateList);

    HRESULT OnPreservedKey(REFGUID rguid, BOOL* isEaten, ITfThreadMgr* threadMgr, TfClientId clientId);

    void SetupDictionaryFile(HINSTANCE dllInstanceHandle, const CRustStringRange& dictionaryFileName);

    void ModifiersUpdate(WPARAM w, LPARAM l);
    bool ModifiersIsShiftKeyDownOnly() const;
    bool ModifiersIsControlKeyDownOnly() const;
    bool ModifiersIsAltKeyDownOnly() const;

    bool PunctuationsHasAlternativePunctuation(WCHAR wch) const;
    WCHAR PunctuationsGetAlternativePunctuationCounted(WCHAR wch);

    HRESULT PreservedKeysInit(ITfThreadMgr* threadMgr, TfClientId clientId);

    void* CompartmentWrapperRawPtr();
    void ConversionModeCompartmentUpdated(ITfThreadMgr* threadMgr);

    void HideLanguageBarButton(bool hide);
    void DisableLanguageBarButton(bool disable);
};

