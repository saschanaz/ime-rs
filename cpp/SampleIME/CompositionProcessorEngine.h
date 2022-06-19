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

class CCompositionProcessorEngine
{
public:
    CCompositionProcessorEngine(LANGID langid, REFGUID guidLanguageProfile, ITfThreadMgr* threadMgr, TfClientId clientId);
    ~CCompositionProcessorEngine() = default;

    BOOL SetupLanguageProfile(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);

    std::tuple<bool, KeystrokeCategory, KeystrokeFunction> TestVirtualKey(uint16_t uCode, char16_t wch, bool fComposing, CandidateMode candidateMode);

    BOOL AddVirtualKey(WCHAR wch);
    void PopVirtualKey();
    void PurgeVirtualKey();

    bool HasVirtualKey();

    std::optional<std::tuple<CRustStringRange, bool>> GetReadingString();
    void GetCandidateList(_Inout_ CSampleImeArray<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch);
    void GetCandidateStringInConverted(const CRustStringRange& searchString, _In_ CSampleImeArray<CCandidateListItem> *pCandidateList);

    // Preserved key handler
    void OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);

    // Punctuation
    bool IsPunctuation(wchar_t wch);
    wchar_t GetPunctuation(wchar_t wch);

    // Language bar control
    void SetLanguageBarStatus(DWORD status, BOOL isSet);

    void ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr);

    void HideLanguageBarButton(bool hide) { engine_rust.HideLanguageBarButton(hide); };
    void DisableLanguageBarButton(bool disable) { engine_rust.DisableLanguageBarButton(disable); };

    void ModifiersUpdate(WPARAM w, LPARAM l) { return engine_rust.ModifiersUpdate(w, l); }

    // TODO: Ultimately split these fields as tuple<langid, profile, engine>,
    // since these fields are not used within this class.
    // For now let's keep them as fields to reduce code change.
    const LANGID langid;
    const GUID guidProfile;

private:
    // Rust port
    class CRustCompositionProcessorEngine {
        void* engine;
    public:
        CRustCompositionProcessorEngine(ITfThreadMgr* threadMgr, TfClientId clientId);
        ~CRustCompositionProcessorEngine();

        bool SetupLanguageProfile(ITfThreadMgr *threadMgr, TfClientId clientId);

        std::tuple<bool, KeystrokeCategory, KeystrokeFunction> TestVirtualKey(uint16_t code, char16_t ch, bool composing, CandidateMode candidateMode);

        bool AddVirtualKey(char16_t wch);
        void PopVirtualKey();
        void PurgeVirtualKey();
        bool HasVirtualKey();
        CRustStringRange KeystrokeBufferGetReadingString();
        bool KeystrokeBufferIncludesWildcard();

        void GetCandidateList(CSampleImeArray<CCandidateListItem> *pCandidateList, bool isIncrementalWordSearch, bool isWildcardSearch);
        void GetCandidateStringInConverted(const CRustStringRange& searchString, CSampleImeArray<CCandidateListItem> *pCandidateList);

        HRESULT OnPreservedKey(REFGUID rguid, bool* isEaten, ITfThreadMgr* threadMgr, TfClientId clientId);

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

    CRustCompositionProcessorEngine engine_rust;
};

