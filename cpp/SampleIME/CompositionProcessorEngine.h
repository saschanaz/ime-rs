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
#include "FileMapping.h"
#include "Compartment.h"
#include "Define.h"
#include "RustStringRange.h"

class CCompositionProcessorEngine
{
public:
    CCompositionProcessorEngine(void);
    ~CCompositionProcessorEngine(void);

    BOOL SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode);

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

    BOOL IsVirtualKeyNeed(UINT uCode, _In_reads_(1) WCHAR *pwch, BOOL fComposing, CANDIDATE_MODE candidateMode, BOOL hasCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState);

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
    BOOL IsPunctuation(WCHAR wch);
    WCHAR GetPunctuation(WCHAR wch);

    BOOL IsDoubleSingleByte(WCHAR wch);
    BOOL IsWildcard() { return _isWildcard; }
    BOOL IsDisableWildcardAtFirst() { return _isDisableWildcardAtFirst; }
    BOOL IsWildcardChar(WCHAR wch) { return ((IsWildcardOneChar(wch) || IsWildcardAllChar(wch)) ? TRUE : FALSE); }
    BOOL IsWildcardOneChar(WCHAR wch) { return (wch==L'?' ? TRUE : FALSE); }
    BOOL IsWildcardAllChar(WCHAR wch) { return (wch==L'*' ? TRUE : FALSE); }
    BOOL IsMakePhraseFromText() { return _hasMakePhraseFromText; }
    BOOL IsKeystrokeSort() { return _isKeystrokeSort; }

    // Dictionary engine
    BOOL IsDictionaryAvailable() { return !!engine_rust.GetTableDictionaryEngine(); }

    // Language bar control
    void SetLanguageBarStatus(DWORD status, BOOL isSet);

    void ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr);

    void ShowAllLanguageBarIcons();
    void HideAllLanguageBarIcons();

    inline CCandidateRange *GetCandidateListIndexRange() { return &_candidateListIndexRange; }
    inline UINT GetCandidateListPhraseModifier() { return _candidateListPhraseModifier; }
    inline UINT GetCandidateWindowWidth() { return _candidateWndWidth; }

private:
    void InitKeyStrokeTable();
    BOOL InitLanguageBar(_In_ CLangBarItemButton *pLanguageBar, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment);

    struct _KEYSTROKE;
    BOOL IsVirtualKeyKeystrokeComposition(UINT uCode, _Out_opt_ _KEYSTROKE_STATE *pKeyState, KEYSTROKE_FUNCTION function);
    BOOL IsKeystrokeRange(UINT uCode, _Out_ _KEYSTROKE_STATE *pKeyState, CANDIDATE_MODE candidateMode);

    void SetupKeystroke();
    void SetupPreserved(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
    void SetupConfiguration();
    void SetupLanguageBar(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode);
    void SetKeystrokeTable(_Inout_ CSampleImeArray<_KEYSTROKE> *pKeystroke);
    void SetupPunctuationPair();
    void CreateLanguageBarButton(DWORD dwEnable, GUID guidLangBar, _In_z_ LPCWSTR pwszDescriptionValue, _In_z_ LPCWSTR pwszTooltipValue, DWORD dwOnIconIndex, DWORD dwOffIconIndex, _Outptr_result_maybenull_ CLangBarItemButton **ppLangBarItemButton, BOOL isSecureMode);
    void SetInitialCandidateListRange();
    void SetDefaultCandidateTextFont();
	void InitializeSampleIMECompartment(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);

    class XPreservedKey;
    void SetPreservedKey(const CLSID clsid, TF_PRESERVEDKEY & tfPreservedKey, _In_z_ LPCWSTR pwszDescription, _Out_ XPreservedKey *pXPreservedKey);
    BOOL InitPreservedKey(_In_ XPreservedKey *pXPreservedKey, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
    BOOL CheckShiftKeyOnly(_In_ CSampleImeArray<TF_PRESERVEDKEY> *pTSFPreservedKeyTable);

    static HRESULT CompartmentCallback(_In_ void *pv, REFGUID guidCompartment);
    void PrivateCompartmentsUpdated(_In_ ITfThreadMgr *pThreadMgr);
    void KeyboardOpenCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr);

private:
    struct _KEYSTROKE
    {
        UINT VirtualKey;
        UINT Modifiers;
        KEYSTROKE_FUNCTION Function;

        _KEYSTROKE()
        {
            VirtualKey = 0;
            Modifiers = 0;
            Function = FUNCTION_NONE;
        }
    };
    _KEYSTROKE _keystrokeTable[26];

    LANGID _langid;
    GUID _guidProfile;
    TfClientId  _tfClientId;

    CSampleImeArray<_KEYSTROKE> _KeystrokeComposition;

    // Preserved key data
    class XPreservedKey
    {
    public:
        XPreservedKey();
        ~XPreservedKey();
        BOOL UninitPreservedKey(_In_ ITfThreadMgr *pThreadMgr);

    public:
        CSampleImeArray<TF_PRESERVEDKEY> TSFPreservedKeyTable;
        GUID Guid;
        LPCWSTR Description;
    };

    XPreservedKey _PreservedKey_IMEMode;
    XPreservedKey _PreservedKey_DoubleSingleByte;
    XPreservedKey _PreservedKey_Punctuation;

    // Punctuation data
    CSampleImeArray<CPunctuationPair> _PunctuationPair;
    CSampleImeArray<CPunctuationNestPair> _PunctuationNestPair;

    // Language bar data
    CLangBarItemButton* _pLanguageBar_IMEMode;
    CLangBarItemButton* _pLanguageBar_DoubleSingleByte;
    CLangBarItemButton* _pLanguageBar_Punctuation;

    // Compartment
    CCompartment* _pCompartmentConversion;
    CCompartmentEventSink* _pCompartmentConversionEventSink;
    CCompartmentEventSink* _pCompartmentKeyboardOpenEventSink;
    CCompartmentEventSink* _pCompartmentDoubleSingleByteEventSink;
    CCompartmentEventSink* _pCompartmentPunctuationEventSink;

    // Configuration data
    bool _isWildcard;
    bool _isDisableWildcardAtFirst;
    bool _hasMakePhraseFromText;
    bool _isKeystrokeSort;
    bool _isComLessMode;
    CCandidateRange _candidateListIndexRange;
    UINT _candidateListPhraseModifier;
    UINT _candidateWndWidth;

    static const int OUT_OF_FILE_INDEX = -1;

    // Rust port
    class CRustCompositionProcessorEngine {
        void* engine;
    public:
        CRustCompositionProcessorEngine();
        ~CRustCompositionProcessorEngine();

        bool AddVirtualKey(char16_t wch);
        void PopVirtualKey();
        void PurgeVirtualKey();
        bool HasVirtualKey();
        CRustStringRange GetReadingString();
        bool KeystrokeBufferIncludesWildcard();

        void SetupDictionaryFile(HINSTANCE dllInstanceHandle, const CRustStringRange& dictionaryFileName, bool isKeystrokeSort);
        std::optional<CRustTableDictionaryEngine> GetTableDictionaryEngine() const;
    };

    CRustCompositionProcessorEngine engine_rust;
};

