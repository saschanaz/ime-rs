// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "SampleIME.h"
#include "CompositionProcessorEngine.h"
#include "LanguageBar.h"
#include "Globals.h"
#include "Compartment.h"
#include "cbindgen/globals.h"
#include "cbindgen/ime.h"
#include "cbindgen/itf_components.h"

//+---------------------------------------------------------------------------
//
// CSampleIME::_UpdateLanguageBarOnSetFocus
//
//----------------------------------------------------------------------------

void CSampleIME::_UpdateLanguageBarOnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus)
{
    BOOL needDisableButtons = FALSE;

    if (!pDocMgrFocus)
    {
        needDisableButtons = TRUE;
    }
    else
    {
        IEnumTfContexts* pEnumContext = nullptr;

        if (FAILED(pDocMgrFocus->EnumContexts(&pEnumContext)) || !pEnumContext)
        {
            needDisableButtons = TRUE;
        }
        else
        {
            ULONG fetched = 0;
            ITfContext* pContext = nullptr;

            if (FAILED(pEnumContext->Next(1, &pContext, &fetched)) || fetched != 1)
            {
                needDisableButtons = TRUE;
            }

            if (!pContext)
            {
                // context is not associated
                needDisableButtons = TRUE;
            }
            else
            {
                pContext->Release();
            }
        }

        if (pEnumContext)
        {
            pEnumContext->Release();
        }
    }

    CCompositionProcessorEngine* pCompositionProcessorEngine = nullptr;
    pCompositionProcessorEngine = _pCompositionProcessorEngine;

    pCompositionProcessorEngine->SetLanguageBarStatus(TF_LBI_STATUS_DISABLED, needDisableButtons);
}

//+---------------------------------------------------------------------------
//
// CCompositionProcessorEngine::SetLanguageBarStatus
//
//----------------------------------------------------------------------------

VOID CCompositionProcessorEngine::SetLanguageBarStatus(DWORD status, BOOL isSet)
{
    if (_pLanguageBar_IMEMode) {
        RustLangBarItemButton::SetStatus(_pLanguageBar_IMEMode, status, isSet);
    }
}

ITfLangBarItemButton* RustLangBarItemButton::New() {
    CRustStringRange langbarImeModeDescription(Global::LangbarImeModeDescription, wcslen(Global::LangbarImeModeDescription));
    CRustStringRange imeModeIconDescription(Global::ImeModeDescription, wcslen(Global::ImeModeDescription));
    return (ITfLangBarItemButton*)langbaritembutton_new(
        &GUID_LBI_INPUTMODE,
        langbarImeModeDescription.GetInternal(),
        imeModeIconDescription.GetInternal(),
        Global::ImeModeOnIcoIndex,
        Global::ImeModeOffIcoIndex
    );
}

HRESULT RustLangBarItemButton::Init(ITfLangBarItemButton* button, ITfThreadMgr* threadMgr, TfClientId clientId, const GUID* guidCompartment) {
    button->AddRef();
    threadMgr->AddRef();
    return langbaritembutton_init(button, threadMgr, clientId, guidCompartment);
}

void RustLangBarItemButton::Cleanup(ITfLangBarItemButton* button) {
    button->AddRef();
    langbaritembutton_cleanup(button);
}

HRESULT RustLangBarItemButton::SetStatus(ITfLangBarItemButton* button, DWORD status, BOOL fSet) {
    button->AddRef();
    return langbaritembutton_set_status(button, status, fSet);
}
