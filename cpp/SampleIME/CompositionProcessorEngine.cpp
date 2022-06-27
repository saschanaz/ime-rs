// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "SampleIME.h"
#include "CompositionProcessorEngine.h"
#include "TfInputProcessorProfile.h"
#include "Globals.h"
#include "Compartment.h"
#include "cbindgen/globals.h"
#include "cbindgen/ime.h"
#include "cbindgen/itf_components.h"

//////////////////////////////////////////////////////////////////////
//
// CSampleIME implementation.
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// _AddTextProcessorEngine
//
//----------------------------------------------------------------------------

BOOL CSampleIME::_AddTextProcessorEngine()
{
    LANGID langid = 0;
    CLSID clsid = GUID_NULL;
    GUID guidProfile = GUID_NULL;

    // Get default profile.
    CTfInputProcessorProfile profile;

    if (FAILED(profile.CreateInstance()))
    {
        return FALSE;
    }

    if (FAILED(profile.GetCurrentLanguage(&langid)))
    {
        return FALSE;
    }

    if (FAILED(profile.GetDefaultLanguageProfile(langid, GUID_TFCAT_TIP_KEYBOARD, &clsid, &guidProfile)))
    {
        return FALSE;
    }

    // Is this already added?
    if (_pCompositionProcessorEngine != nullptr)
    {
        LANGID langidProfile = _pCompositionProcessorEngine->langid;
        GUID guidLanguageProfile = _pCompositionProcessorEngine->guidProfile;

        if ((langid == langidProfile) && IsEqualGUID(guidProfile, guidLanguageProfile))
        {
            return TRUE;
        }
    }

    // Create composition processor engine
    if (_pCompositionProcessorEngine == nullptr)
    {
        _pCompositionProcessorEngine = new (std::nothrow) CCompositionProcessorEngine(langid, guidProfile, _GetThreadMgr(), _GetClientId());
    }
    if (!_pCompositionProcessorEngine)
    {
        return FALSE;
    }

    // setup composition processor engine
    if (FALSE == _pCompositionProcessorEngine->SetupLanguageProfile(_GetThreadMgr(), _GetClientId()))
    {
        return FALSE;
    }

    return TRUE;
}

//////////////////////////////////////////////////////////////////////
//
// CompositionProcessorEngine implementation.
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// ctor
// param
//     [in] LANGID langid = Specify language ID
//     [in] GUID guidLanguageProfile - Specify GUID language profile which GUID is as same as Text Service Framework language profile.
//
//----------------------------------------------------------------------------

CCompositionProcessorEngine::CCompositionProcessorEngine(
    LANGID langid,
    REFGUID guidLanguageProfile,
    ITfThreadMgr *threadMgr,
    TfClientId clientId
) : langid(langid), guidProfile(guidLanguageProfile) {
    threadMgr->AddRef();
    engine = compositionprocessorengine_new(threadMgr, clientId);
}

CCompositionProcessorEngine::~CCompositionProcessorEngine() {
    compositionprocessorengine_free(engine);
}

bool CCompositionProcessorEngine::SetupLanguageProfile(ITfThreadMgr *threadMgr, TfClientId clientId) {
    return compositionprocessorengine_setup_language_profile(engine, threadMgr, clientId);
};

bool CCompositionProcessorEngine::AddVirtualKey(char16_t wch) {
    return compositionprocessorengine_add_virtual_key(engine, wch);
}

void CCompositionProcessorEngine::PopVirtualKey() {
    compositionprocessorengine_pop_virtual_key(engine);
}

void CCompositionProcessorEngine::PurgeVirtualKey() {
    compositionprocessorengine_purge_virtual_key(engine);
}

bool CCompositionProcessorEngine::HasVirtualKey() {
    return compositionprocessorengine_has_virtual_key(engine);
}

CRustStringRange CCompositionProcessorEngine::KeystrokeBufferGetReadingString() {
    void* str = compositionprocessorengine_keystroke_buffer_get_reading_string(engine);
    return CRustStringRange::FromVoid(str);
}

bool CCompositionProcessorEngine::KeystrokeBufferIncludesWildcard() {
    return compositionprocessorengine_keystroke_buffer_includes_wildcard(engine);
}

static const uintptr_t MAX_BUFFER = 512;

inline void ArraysToArray(void** keys, void** values, uintptr_t length, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList) {
    for (uintptr_t i = 0; i < length; i++) {
        CRustStringRange key = CRustStringRange::FromVoid(keys[i]);
        CRustStringRange value = CRustStringRange::FromVoid(values[i]);
        CCandidateListItem listItem(value, key);
        pItemList->Append(listItem);
    }
}

void CCompositionProcessorEngine::GetCandidateList(CSampleImeArray<CCandidateListItem> *pCandidateList, bool isIncrementalWordSearch, bool isWildcardSearch) {
    void* keys[MAX_BUFFER];
    void* values[MAX_BUFFER];
    uintptr_t length = compositionprocessorengine_get_candidate_list(this->engine, keys, values, MAX_BUFFER, isIncrementalWordSearch, isWildcardSearch);
    ArraysToArray(keys, values, length, pCandidateList);
}

void CCompositionProcessorEngine::GetCandidateStringInConverted(const CRustStringRange& searchString, CSampleImeArray<CCandidateListItem> *pCandidateList) {
    void* keys[MAX_BUFFER];
    void* values[MAX_BUFFER];
    uintptr_t length = compositionprocessorengine_get_candidate_string_in_converted(engine, searchString.GetInternal(), keys, values, MAX_BUFFER);
    ArraysToArray(keys, values, length, pCandidateList);
}

HRESULT CCompositionProcessorEngine::OnPreservedKey(REFGUID rguid, BOOL* isEaten, ITfThreadMgr* threadMgr, TfClientId clientId) {
    threadMgr->AddRef();
    bool _isEaten = false;
    HRESULT result = compositionprocessorengine_on_preserved_key(engine, &rguid, &_isEaten, threadMgr, clientId);
    *isEaten = _isEaten;
    return result;
}

void CCompositionProcessorEngine::ModifiersUpdate(WPARAM w, LPARAM l) {
    compositionprocessorengine_modifiers_update(engine, w, l);
}

bool CCompositionProcessorEngine::ModifiersIsShiftKeyDownOnly() const {
    return compositionprocessorengine_modifiers_is_shift_key_down_only(engine);
}

bool CCompositionProcessorEngine::ModifiersIsControlKeyDownOnly() const {
    return compositionprocessorengine_modifiers_is_control_key_down_only(engine);
}

bool CCompositionProcessorEngine::ModifiersIsAltKeyDownOnly() const {
    return compositionprocessorengine_modifiers_is_alt_key_down_only(engine);
}

bool CCompositionProcessorEngine::PunctuationsHasAlternativePunctuation(WCHAR wch) const {
    return compositionprocessorengine_punctuations_has_alternative_punctuation(engine, wch);
}

wchar_t CCompositionProcessorEngine::PunctuationsGetAlternativePunctuationCounted(wchar_t wch) {
    return compositionprocessorengine_punctuations_get_alternative_punctuation_counted(engine, wch);
}

HRESULT CCompositionProcessorEngine::PreservedKeysInit(ITfThreadMgr* threadMgr, TfClientId clientId) {
    threadMgr->AddRef();
    return compositionprocessorengine_preserved_keys_init(engine, threadMgr, clientId);
}

void* CCompositionProcessorEngine::CompartmentWrapperRawPtr() {
    return const_cast<void*>(compositionprocessorengine_compartmentwrapper_raw_ptr(engine));
}

void CCompositionProcessorEngine::ConversionModeCompartmentUpdated(ITfThreadMgr *threadMgr) {
    threadMgr->AddRef();
    compositionprocessorengine_compartmentwrapper_conversion_mode_compartment_updated(engine, threadMgr);
}

void CCompositionProcessorEngine::HideLanguageBarButton(bool hide) {
    compositionprocessorengine_hide_language_bar_button(engine, hide);
}

void CCompositionProcessorEngine::DisableLanguageBarButton(bool disable) {
    compositionprocessorengine_disable_language_bar_button(engine, disable);
}
