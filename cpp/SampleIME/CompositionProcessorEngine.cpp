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
        LANGID langidProfile = 0;
        GUID guidLanguageProfile = GUID_NULL;

        guidLanguageProfile = _pCompositionProcessorEngine->GetLanguageProfile(&langidProfile);
        if ((langid == langidProfile) && IsEqualGUID(guidProfile, guidLanguageProfile))
        {
            return TRUE;
        }
    }

    // Create composition processor engine
    if (_pCompositionProcessorEngine == nullptr)
    {
        _pCompositionProcessorEngine = new (std::nothrow) CCompositionProcessorEngine(_GetThreadMgr(), _GetClientId());
    }
    if (!_pCompositionProcessorEngine)
    {
        return FALSE;
    }

    // setup composition processor engine
    if (FALSE == _pCompositionProcessorEngine->SetupLanguageProfile(langid, guidProfile, _GetThreadMgr(), _GetClientId()))
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
//
//----------------------------------------------------------------------------

CCompositionProcessorEngine::CCompositionProcessorEngine(ITfThreadMgr *threadMgr, TfClientId clientId)
    : engine_rust(threadMgr, clientId)
{
    _langid = 0xffff;
    _guidProfile = GUID_NULL;
}

//+---------------------------------------------------------------------------
//
// SetupLanguageProfile
//
// Setup language profile for Composition Processor Engine.
// param
//     [in] LANGID langid = Specify language ID
//     [in] GUID guidLanguageProfile - Specify GUID language profile which GUID is as same as Text Service Framework language profile.
//     [in] ITfThreadMgr - pointer ITfThreadMgr.
//     [in] tfClientId - TfClientId value.
// returns
//     If setup succeeded, returns true. Otherwise returns false.
// N.B. For reverse conversion, ITfThreadMgr is NULL and TfClientId is 0.
//+---------------------------------------------------------------------------

BOOL CCompositionProcessorEngine::SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    BOOL ret = TRUE;
    if ((tfClientId == 0) && (pThreadMgr == nullptr))
    {
        ret = FALSE;
        goto Exit;
    }

    _langid = langid;
    _guidProfile = guidLanguageProfile;

    engine_rust.SetupLanguageProfile(pThreadMgr, tfClientId);

Exit:
    return ret;
}

//+---------------------------------------------------------------------------
//
// AddVirtualKey
// Add virtual key code to Composition Processor Engine for used to parse keystroke data.
// param
//     [in] uCode - Specify virtual key code.
// returns
//     State of Text Processor Engine.
//----------------------------------------------------------------------------

BOOL CCompositionProcessorEngine::AddVirtualKey(WCHAR wch)
{
    return engine_rust.AddVirtualKey(wch);
}

//+---------------------------------------------------------------------------
//
// PopVirtualKey
// Remove the last stored virtual key code.
// returns
//     none.
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::PopVirtualKey()
{
    engine_rust.PopVirtualKey();
}

//+---------------------------------------------------------------------------
//
// PurgeVirtualKey
// Purge stored virtual key code.
// param
//     none.
// returns
//     none.
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::PurgeVirtualKey()
{
    engine_rust.PurgeVirtualKey();
}

bool CCompositionProcessorEngine::HasVirtualKey()
{
    return engine_rust.HasVirtualKey();
}

//+---------------------------------------------------------------------------
//
// GetReadingString
// Retrieves string from Composition Processor Engine.
//
//----------------------------------------------------------------------------

std::optional<std::tuple<CRustStringRange, bool>> CCompositionProcessorEngine::GetReadingString()
{
    if (engine_rust.HasVirtualKey())
    {
        return std::tuple<CRustStringRange, bool>(engine_rust.KeystrokeBufferGetReadingString(), engine_rust.KeystrokeBufferIncludesWildcard());
    }

    return std::nullopt;
}

//+---------------------------------------------------------------------------
//
// GetCandidateList
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::GetCandidateList(_Inout_ CSampleImeArray<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch)
{
    engine_rust.GetCandidateList(pCandidateList, isIncrementalWordSearch, isWildcardSearch);
}

//+---------------------------------------------------------------------------
//
// GetCandidateStringInConverted
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::GetCandidateStringInConverted(const CRustStringRange& searchString, _In_ CSampleImeArray<CCandidateListItem> *pCandidateList)
{
    engine_rust.GetCandidateStringInConverted(searchString, pCandidateList);
}

//+---------------------------------------------------------------------------
//
// IsPunctuation
//
//----------------------------------------------------------------------------

bool CCompositionProcessorEngine::IsPunctuation(wchar_t wch)
{
    return engine_rust.PunctuationsHasAlternativePunctuation(wch);
}

//+---------------------------------------------------------------------------
//
// GetPunctuationPair
//
//----------------------------------------------------------------------------

wchar_t CCompositionProcessorEngine::GetPunctuation(wchar_t wch)
{
    return engine_rust.PunctuationsGetAlternativePunctuationCounted(wch);
}

//+---------------------------------------------------------------------------
//
// OnPreservedKey
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    bool isEaten;
    engine_rust.OnPreservedKey(rguid, &isEaten, pThreadMgr, tfClientId);
    *pIsEaten = isEaten;
}

//+---------------------------------------------------------------------------
//
// UpdatePrivateCompartments
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr)
{
    engine_rust.ConversionModeCompartmentUpdated(pThreadMgr);
}

//////////////////////////////////////////////////////////////////////
//
//    CCompositionProcessorEngine
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// CCompositionProcessorEngine::IsVirtualKeyNeed
//
// Test virtual key code need to the Composition Processor Engine.
// param
//     [in] uCode - Specify virtual key code.
//     [in/out] pwch       - char code
//     [in] fComposing     - Specified composing.
//     [in] fCandidateMode - Specified candidate mode.
//     [out] pKeyState     - Returns function regarding virtual key.
// returns
//     If engine need this virtual key code, returns true. Otherwise returns false.
//----------------------------------------------------------------------------

std::tuple<bool, KeystrokeCategory, KeystrokeFunction> CCompositionProcessorEngine::TestVirtualKey(uint16_t uCode, char16_t wch, bool fComposing, CandidateMode candidateMode)
{
    return engine_rust.TestVirtualKey(uCode, wch, fComposing, candidateMode);
}

CCompositionProcessorEngine::CRustCompositionProcessorEngine::CRustCompositionProcessorEngine(ITfThreadMgr *threadMgr, TfClientId clientId) {
    threadMgr->AddRef();
    engine = compositionprocessorengine_new(threadMgr, clientId);
}

CCompositionProcessorEngine::CRustCompositionProcessorEngine::~CRustCompositionProcessorEngine() {
    compositionprocessorengine_free(engine);
}

bool CCompositionProcessorEngine::CRustCompositionProcessorEngine::SetupLanguageProfile(ITfThreadMgr *threadMgr, TfClientId clientId) {
    return compositionprocessorengine_setup_language_profile(engine, threadMgr, clientId);
};

std::tuple<bool, KeystrokeCategory, KeystrokeFunction> CCompositionProcessorEngine::CRustCompositionProcessorEngine::TestVirtualKey(uint16_t code, char16_t ch, bool composing, CandidateMode candidateMode)
{
    bool keyEaten;
    KeystrokeCategory keystrokeCategory;
    KeystrokeFunction keystrokeFunction;
    compositionprocessorengine_test_virtual_key(engine, code, ch, composing, candidateMode, &keyEaten, &keystrokeCategory, &keystrokeFunction);
    return { keyEaten, keystrokeCategory, keystrokeFunction };
}

bool CCompositionProcessorEngine::CRustCompositionProcessorEngine::AddVirtualKey(char16_t wch) {
    return compositionprocessorengine_add_virtual_key(engine, wch);
}

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::PopVirtualKey() {
    compositionprocessorengine_pop_virtual_key(engine);
}

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::PurgeVirtualKey() {
    compositionprocessorengine_purge_virtual_key(engine);
}

bool CCompositionProcessorEngine::CRustCompositionProcessorEngine::HasVirtualKey() {
    return compositionprocessorengine_has_virtual_key(engine);
}

CRustStringRange CCompositionProcessorEngine::CRustCompositionProcessorEngine::KeystrokeBufferGetReadingString() {
    void* str = compositionprocessorengine_keystroke_buffer_get_reading_string(engine);
    return CRustStringRange::FromVoid(str);
}

bool CCompositionProcessorEngine::CRustCompositionProcessorEngine::KeystrokeBufferIncludesWildcard() {
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

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::GetCandidateList(CSampleImeArray<CCandidateListItem> *pCandidateList, bool isIncrementalWordSearch, bool isWildcardSearch) {
    void* keys[MAX_BUFFER];
    void* values[MAX_BUFFER];
    uintptr_t length = compositionprocessorengine_get_candidate_list(this->engine, keys, values, MAX_BUFFER, isIncrementalWordSearch, isWildcardSearch);
    ArraysToArray(keys, values, length, pCandidateList);
}

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::GetCandidateStringInConverted(const CRustStringRange& searchString, CSampleImeArray<CCandidateListItem> *pCandidateList) {
    void* keys[MAX_BUFFER];
    void* values[MAX_BUFFER];
    uintptr_t length = compositionprocessorengine_get_candidate_string_in_converted(engine, searchString.GetInternal(), keys, values, MAX_BUFFER);
    ArraysToArray(keys, values, length, pCandidateList);
}

HRESULT CCompositionProcessorEngine::CRustCompositionProcessorEngine::OnPreservedKey(REFGUID rguid, bool* isEaten, ITfThreadMgr* threadMgr, TfClientId clientId) {
    threadMgr->AddRef();
    return compositionprocessorengine_on_preserved_key(engine, &rguid, isEaten, threadMgr, clientId);
}

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::ModifiersUpdate(WPARAM w, LPARAM l) {
    compositionprocessorengine_modifiers_update(engine, w, l);
}

bool CCompositionProcessorEngine::CRustCompositionProcessorEngine::ModifiersIsShiftKeyDownOnly() const {
    return compositionprocessorengine_modifiers_is_shift_key_down_only(engine);
}

bool CCompositionProcessorEngine::CRustCompositionProcessorEngine::ModifiersIsControlKeyDownOnly() const {
    return compositionprocessorengine_modifiers_is_control_key_down_only(engine);
}

bool CCompositionProcessorEngine::CRustCompositionProcessorEngine::ModifiersIsAltKeyDownOnly() const {
    return compositionprocessorengine_modifiers_is_alt_key_down_only(engine);
}

bool CCompositionProcessorEngine::CRustCompositionProcessorEngine::PunctuationsHasAlternativePunctuation(WCHAR wch) const {
    return compositionprocessorengine_punctuations_has_alternative_punctuation(engine, wch);
}

wchar_t CCompositionProcessorEngine::CRustCompositionProcessorEngine::PunctuationsGetAlternativePunctuationCounted(wchar_t wch) {
    return compositionprocessorengine_punctuations_get_alternative_punctuation_counted(engine, wch);
}

HRESULT CCompositionProcessorEngine::CRustCompositionProcessorEngine::PreservedKeysInit(ITfThreadMgr* threadMgr, TfClientId clientId) {
    threadMgr->AddRef();
    return compositionprocessorengine_preserved_keys_init(engine, threadMgr, clientId);
}

void* CCompositionProcessorEngine::CRustCompositionProcessorEngine::CompartmentWrapperRawPtr() {
    return const_cast<void*>(compositionprocessorengine_compartmentwrapper_raw_ptr(engine));
}

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::ConversionModeCompartmentUpdated(ITfThreadMgr *threadMgr) {
    threadMgr->AddRef();
    compositionprocessorengine_compartmentwrapper_conversion_mode_compartment_updated(engine, threadMgr);
}

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::HideLanguageBarButton(bool hide) {
    compositionprocessorengine_hide_language_bar_button(engine, hide);
}

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::DisableLanguageBarButton(bool disable) {
    compositionprocessorengine_disable_language_bar_button(engine, disable);
}
