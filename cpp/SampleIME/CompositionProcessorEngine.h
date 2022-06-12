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
#include "TableDictionaryEngine.h"
#include "KeyHandlerEditSession.h"
#include "SampleIMEBaseStructure.h"
#include "Compartment.h"
#include "Define.h"
#include "RustStringRange.h"

class CCompositionProcessorEngine
{
public:
    CCompositionProcessorEngine(ITfThreadMgr* threadMgr, TfClientId clientId);
    ~CCompositionProcessorEngine() = default;

    BOOL SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);

    // Get language profile.
    GUID GetLanguageProfile(LANGID *plangid)
    {
        *plangid = _langid;
        return _guidProfile;
    }
    // Get locale
    LCID GetLocale()
    {
        return MAKELCID(_langid, SORT_DEFAULT);
    }

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

    BOOL IsDoubleSingleByte(WCHAR wch);
    BOOL IsWildcardChar(WCHAR wch) { return (wch == u'?' || wch == u'*'); }

    // Dictionary engine
    BOOL IsDictionaryAvailable() { return !!engine_rust.GetTableDictionaryEngine(); }

    // Language bar control
    void SetLanguageBarStatus(DWORD status, BOOL isSet);

    void ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr);

    void ShowAllLanguageBarIcons();
    void HideAllLanguageBarIcons();

    void ModifiersUpdate(WPARAM w, LPARAM l) { return engine_rust.ModifiersUpdate(w, l); }

    const uint32_t CandidateWindowWidth = 13;  // * tmMaxCharWidth

private:
	void InitializeSampleIMECompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);

    void PrivateCompartmentsUpdated(_In_ ITfThreadMgr *pThreadMgr);

private:
    LANGID _langid;
    GUID _guidProfile;
    TfClientId  _tfClientId;

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
        CRustStringRange GetReadingString();
        bool KeystrokeBufferIncludesWildcard();

        HRESULT OnPreservedKey(REFGUID rguid, bool* isEaten, ITfThreadMgr* threadMgr, TfClientId clientId);

        void SetupDictionaryFile(HINSTANCE dllInstanceHandle, const CRustStringRange& dictionaryFileName);
        std::optional<CRustTableDictionaryEngine> GetTableDictionaryEngine() const;

        void ModifiersUpdate(WPARAM w, LPARAM l);
        bool ModifiersIsShiftKeyDownOnly() const;
        bool ModifiersIsControlKeyDownOnly() const;
        bool ModifiersIsAltKeyDownOnly() const;

        bool PunctuationsHasAlternativePunctuation(WCHAR wch) const;
        WCHAR PunctuationsGetAlternativePunctuationCounted(WCHAR wch);

        HRESULT PreservedKeysInit(ITfThreadMgr* threadMgr, TfClientId clientId);

        void* CompartmentWrapperRawPtr();
        void ConversionModeCompartmentUpdated(ITfThreadMgr* threadMgr);
        void PrivateCompartmentsUpdated(ITfThreadMgr* threadMgr);

        void SetLanguageBarStatus(uint32_t status, bool set);
    };

    CRustCompositionProcessorEngine engine_rust;
};

