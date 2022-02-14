﻿// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
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
#include "LanguageBar.h"
#include "cbindgen/globals.h"
#include "cbindgen/ime.h"

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
    _tfClientId = TF_CLIENTID_NULL;

    _pLanguageBar_IMEMode = nullptr;

    _pCompartmentConversion = nullptr;
    _pCompartmentKeyboardOpenEventSink = nullptr;
    _pCompartmentConversionEventSink = nullptr;
    _pCompartmentDoubleSingleByteEventSink = nullptr;
    _pCompartmentPunctuationEventSink = nullptr;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CCompositionProcessorEngine::~CCompositionProcessorEngine()
{
    if (_pLanguageBar_IMEMode)
    {
        _pLanguageBar_IMEMode->CleanUp();
        _pLanguageBar_IMEMode->Release();
        _pLanguageBar_IMEMode = nullptr;
    }

    if (_pCompartmentConversion)
    {
        delete _pCompartmentConversion;
        _pCompartmentConversion = nullptr;
    }
    if (_pCompartmentKeyboardOpenEventSink)
    {
        _pCompartmentKeyboardOpenEventSink->_Unadvise();
        delete _pCompartmentKeyboardOpenEventSink;
        _pCompartmentKeyboardOpenEventSink = nullptr;
    }
    if (_pCompartmentConversionEventSink)
    {
        _pCompartmentConversionEventSink->_Unadvise();
        delete _pCompartmentConversionEventSink;
        _pCompartmentConversionEventSink = nullptr;
    }
    if (_pCompartmentDoubleSingleByteEventSink)
    {
        _pCompartmentDoubleSingleByteEventSink->_Unadvise();
        delete _pCompartmentDoubleSingleByteEventSink;
        _pCompartmentDoubleSingleByteEventSink = nullptr;
    }
    if (_pCompartmentPunctuationEventSink)
    {
        _pCompartmentPunctuationEventSink->_Unadvise();
        delete _pCompartmentPunctuationEventSink;
        _pCompartmentPunctuationEventSink = nullptr;
    }
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
    _tfClientId = tfClientId;

    engine_rust.PreservedKeysInit(pThreadMgr, tfClientId);
	InitializeSampleIMECompartment(pThreadMgr, tfClientId);
    SetupLanguageBar(pThreadMgr, tfClientId);
    SetDefaultCandidateTextFont();
    engine_rust.SetupDictionaryFile(DLL_INSTANCE, TEXTSERVICE_DIC);

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
        return std::tuple<CRustStringRange, bool>(engine_rust.GetReadingString(), engine_rust.KeystrokeBufferIncludesWildcard());
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
    if (!IsDictionaryAvailable())
    {
        return;
    }

    if (isIncrementalWordSearch)
    {
        CRustStringRange wildcardSearch = engine_rust.GetReadingString();

        // check keystroke buffer already has wildcard char which end user want wildcard serach
        uint32_t wildcardIndex = 0;
        bool isFindWildcard = wildcardSearch.Contains(u8'*') || wildcardSearch.Contains(u8'?');

        if (!isFindWildcard)
        {
            // add wildcard char for incremental search
            wildcardSearch = wildcardSearch.Concat(u8"*"_rs);
        }

        engine_rust.GetTableDictionaryEngine()->CollectWordForWildcard(wildcardSearch, pCandidateList);

        if (0 >= pCandidateList->Count())
        {
            return;
        }
    }
    else if (isWildcardSearch)
    {
        engine_rust.GetTableDictionaryEngine()->CollectWordForWildcard(engine_rust.GetReadingString(), pCandidateList);
    }
    else
    {
        engine_rust.GetTableDictionaryEngine()->CollectWord(engine_rust.GetReadingString(), pCandidateList);
    }
}

//+---------------------------------------------------------------------------
//
// GetCandidateStringInConverted
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::GetCandidateStringInConverted(const CRustStringRange& searchString, _In_ CSampleImeArray<CCandidateListItem> *pCandidateList)
{
    if (!IsDictionaryAvailable())
    {
        return;
    }

    // Search phrase from SECTION_TEXT's converted string list
    CRustStringRange wildcardSearch = searchString.Concat(u8"*"_rs);

    engine_rust.GetTableDictionaryEngine()->CollectWordFromConvertedStringForWildcard(wildcardSearch, pCandidateList);
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
// IsDoubleSingleByte
//
//----------------------------------------------------------------------------

BOOL CCompositionProcessorEngine::IsDoubleSingleByte(WCHAR wch)
{
    if (L' ' <= wch && wch <= L'~')
    {
        return TRUE;
    }
    return FALSE;
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
// SetupLanguageBar
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::SetupLanguageBar(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    DWORD dwEnable = 1;
    CreateLanguageBarButton(dwEnable, GUID_LBI_INPUTMODE, Global::LangbarImeModeDescription, Global::ImeModeDescription, Global::ImeModeOnIcoIndex, Global::ImeModeOffIcoIndex, &_pLanguageBar_IMEMode);

    InitLanguageBar(_pLanguageBar_IMEMode, pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);

    _pCompartmentConversion = new (std::nothrow) CCompartment(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
    _pCompartmentKeyboardOpenEventSink = new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this);
    _pCompartmentConversionEventSink = new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this);
    _pCompartmentDoubleSingleByteEventSink = new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this);
    _pCompartmentPunctuationEventSink = new (std::nothrow) CCompartmentEventSink(CompartmentCallback, this);

    if (_pCompartmentKeyboardOpenEventSink)
    {
        _pCompartmentKeyboardOpenEventSink->_Advise(pThreadMgr, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
    }
    if (_pCompartmentConversionEventSink)
    {
        _pCompartmentConversionEventSink->_Advise(pThreadMgr, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
    }
    if (_pCompartmentDoubleSingleByteEventSink)
    {
        _pCompartmentDoubleSingleByteEventSink->_Advise(pThreadMgr, SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE);
    }
    if (_pCompartmentPunctuationEventSink)
    {
        _pCompartmentPunctuationEventSink->_Advise(pThreadMgr, SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION);
    }

    return;
}

//+---------------------------------------------------------------------------
//
// CreateLanguageBarButton
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::CreateLanguageBarButton(DWORD dwEnable, GUID guidLangBar, _In_z_ LPCWSTR pwszDescriptionValue, _In_z_ LPCWSTR pwszTooltipValue, DWORD dwOnIconIndex, DWORD dwOffIconIndex, _Outptr_result_maybenull_ CLangBarItemButton **ppLangBarItemButton)
{
	dwEnable;

    if (ppLangBarItemButton)
    {
        *ppLangBarItemButton = new (std::nothrow) CLangBarItemButton(guidLangBar, pwszDescriptionValue, pwszTooltipValue, dwOnIconIndex, dwOffIconIndex);
    }

    return;
}

//+---------------------------------------------------------------------------
//
// InitLanguageBar
//
//----------------------------------------------------------------------------

BOOL CCompositionProcessorEngine::InitLanguageBar(_In_ CLangBarItemButton *pLangBarItemButton, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment)
{
    if (pLangBarItemButton)
    {
        if (pLangBarItemButton->_AddItem(pThreadMgr) == S_OK)
        {
            if (pLangBarItemButton->_RegisterCompartment(pThreadMgr, tfClientId, guidCompartment))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void CCompositionProcessorEngine::InitializeSampleIMECompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
	// set initial mode
    CCompartment CompartmentKeyboardOpen(pThreadMgr, tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
    CompartmentKeyboardOpen._SetCompartmentBOOL(TRUE);

    CCompartment CompartmentDoubleSingleByte(pThreadMgr, tfClientId, SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE);
    CompartmentDoubleSingleByte._SetCompartmentBOOL(FALSE);

    CCompartment CompartmentPunctuation(pThreadMgr, tfClientId, SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION);
    CompartmentPunctuation._SetCompartmentBOOL(TRUE);

    PrivateCompartmentsUpdated(pThreadMgr);
}
//+---------------------------------------------------------------------------
//
// CompartmentCallback
//
//----------------------------------------------------------------------------

// static
HRESULT CCompositionProcessorEngine::CompartmentCallback(_In_ void *pv, REFGUID guidCompartment)
{
    CCompositionProcessorEngine* fakeThis = (CCompositionProcessorEngine*)pv;
    if (nullptr == fakeThis)
    {
        return E_INVALIDARG;
    }

    ITfThreadMgr* pThreadMgr = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**)&pThreadMgr);
    if (FAILED(hr))
    {
        return E_FAIL;
    }

    if (IsEqualGUID(guidCompartment, SAMPLEIME_GUID_COMPARTMENT_DOUBLE_SINGLE_BYTE) ||
        IsEqualGUID(guidCompartment, SAMPLEIME_GUID_COMPARTMENT_PUNCTUATION))
    {
        fakeThis->PrivateCompartmentsUpdated(pThreadMgr);
    }
    else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION) ||
        IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE))
    {
        fakeThis->ConversionModeCompartmentUpdated(pThreadMgr);
    }
    else if (IsEqualGUID(guidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
    {
        fakeThis->KeyboardOpenCompartmentUpdated(pThreadMgr);
    }

    pThreadMgr->Release();
    pThreadMgr = nullptr;

    return S_OK;
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

//+---------------------------------------------------------------------------
//
// PrivateCompartmentsUpdated()
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::PrivateCompartmentsUpdated(_In_ ITfThreadMgr *pThreadMgr)
{
    engine_rust.PrivateCompartmentsUpdated(pThreadMgr);
}

//+---------------------------------------------------------------------------
//
// KeyboardOpenCompartmentUpdated
//
//----------------------------------------------------------------------------

void CCompositionProcessorEngine::KeyboardOpenCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr)
{
    engine_rust.KeyboardOpenCompartmentUpdated(pThreadMgr);
}

void CCompositionProcessorEngine::ShowAllLanguageBarIcons()
{
    SetLanguageBarStatus(TF_LBI_STATUS_HIDDEN, FALSE);
}

void CCompositionProcessorEngine::HideAllLanguageBarIcons()
{
    SetLanguageBarStatus(TF_LBI_STATUS_HIDDEN, TRUE);
}

void CCompositionProcessorEngine::SetDefaultCandidateTextFont()
{
    // Candidate Text Font
    if (Global::defaultlFontHandle == nullptr)
    {
		WCHAR fontName[50] = {'\0'};
		LoadString(DLL_INSTANCE, IDS_DEFAULT_FONT, fontName, 50);
        Global::defaultlFontHandle = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, fontName);
        if (!Global::defaultlFontHandle)
        {
			LOGFONT lf;
			SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
            // Fall back to the default GUI font on failure.
            Global::defaultlFontHandle = CreateFont(-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72), 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, lf.lfFaceName);
        }
    }
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

CRustStringRange CCompositionProcessorEngine::CRustCompositionProcessorEngine::GetReadingString() {
    void* str = compositionprocessorengine_get_reading_string(engine);
    return CRustStringRange::FromVoid(str);
}

bool CCompositionProcessorEngine::CRustCompositionProcessorEngine::KeystrokeBufferIncludesWildcard() {
    return compositionprocessorengine_keystroke_buffer_includes_wildcard(engine);
}

HRESULT CCompositionProcessorEngine::CRustCompositionProcessorEngine::OnPreservedKey(REFGUID rguid, bool* isEaten, ITfThreadMgr* threadMgr, TfClientId clientId) {
    threadMgr->AddRef();
    return compositionprocessorengine_on_preserved_key(engine, &rguid, isEaten, threadMgr, clientId);
}

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::SetupDictionaryFile(HINSTANCE dllInstanceHandle, const CRustStringRange& dictionaryFileName) {
    compositionprocessorengine_setup_dictionary_file(engine, dllInstanceHandle, dictionaryFileName.GetInternal());
}

std::optional<CRustTableDictionaryEngine> CCompositionProcessorEngine::CRustCompositionProcessorEngine::GetTableDictionaryEngine() const {
    const void* dict = compositionprocessorengine_get_table_dictionary_engine(engine);
    if (dict) {
        return CRustTableDictionaryEngine::WeakRef(const_cast<void*>(compositionprocessorengine_get_table_dictionary_engine(engine)));
    }
    return std::nullopt;
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

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::ConversionModeCompartmentUpdated(ITfThreadMgr *threadMgr) {
    threadMgr->AddRef();
    compositionprocessorengine_compartmentwrapper_conversion_mode_compartment_updated(engine, threadMgr);
}

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::PrivateCompartmentsUpdated(ITfThreadMgr *threadMgr) {
    threadMgr->AddRef();
    compositionprocessorengine_compartmentwrapper_private_compartments_updated(engine, threadMgr);
}

void CCompositionProcessorEngine::CRustCompositionProcessorEngine::KeyboardOpenCompartmentUpdated(ITfThreadMgr *threadMgr) {
    threadMgr->AddRef();
    compositionprocessorengine_compartmentwrapper_keyboard_open_compartment_updated(engine, threadMgr);
}
