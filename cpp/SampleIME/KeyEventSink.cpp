// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "SampleIME.h"
#include "CandidateListUIPresenter.h"
#include "CompositionProcessorEngine.h"
#include "KeyHandlerEditSession.h"
#include "Compartment.h"
#include "cbindgen/globals.h"
#include "cbindgen/input_processor.h"

bool IsDoubleSingleByte(char16_t wch)
{
    return u' ' <= wch && wch <= u'~';
}

//+---------------------------------------------------------------------------
//
// _IsKeyEaten
//
//----------------------------------------------------------------------------

BOOL CSampleIME::_IsKeyEaten(UINT codeIn, _Out_writes_(1) WCHAR *pwch, _Out_opt_ _KEYSTROKE_STATE *pKeyState)
{
    _pThreadMgr->AddRef();
    return key_event_sink_is_key_eaten(
        _pThreadMgr,
        _tfClientId,
        _pCompositionProcessorEngine->GetRaw(),
        _IsComposing(),
        _candidateMode,
        codeIn,
        (uint16_t*)pwch,
        &pKeyState->Category,
        &pKeyState->Function
    );
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnSetFocus
//
// Called by the system whenever this service gets the keystroke device focus.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnSetFocus(BOOL fForeground)
{
	fForeground;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnTestKeyDown
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    GetCompositionProcessorEngine()->ModifiersUpdate(wParam, lParam);

    _KEYSTROKE_STATE KeystrokeState;
    WCHAR wch = '\0';
    *pIsEaten = _IsKeyEaten((UINT)wParam, &wch, &KeystrokeState);

    if (KeystrokeState.Category == KeystrokeCategory::InvokeCompositionEditSession)
    {
        //
        // Invoke key handler edit session
        //
        KeystrokeState.Category = KeystrokeCategory::Composing;

        _InvokeKeyHandler(pContext, wch, (DWORD)lParam, KeystrokeState);
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnKeyDown
//
// Called by the system to offer this service a keystroke.  If *pIsEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    GetCompositionProcessorEngine()->ModifiersUpdate(wParam, lParam);

    _KEYSTROKE_STATE KeystrokeState;
    WCHAR wch = '\0';

    *pIsEaten = _IsKeyEaten((UINT)wParam, &wch, &KeystrokeState);

    if (*pIsEaten)
    {
        //
        // Invoke key handler edit session
        //
        if (wParam == VK_ESCAPE)
        {
            KeystrokeState.Category = KeystrokeCategory::Composing;
        }
    }
    else if (KeystrokeState.Category == KeystrokeCategory::InvokeCompositionEditSession)
    {
        // Invoke key handler edit session
        KeystrokeState.Category = KeystrokeCategory::Composing;
    }

    _InvokeKeyHandler(pContext, wch, (DWORD)lParam, KeystrokeState);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnTestKeyUp
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    if (pIsEaten == nullptr)
    {
        return E_INVALIDARG;
    }

    return OnKeyUp(pContext, wParam, lParam, pIsEaten);
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnKeyUp
//
// Called by the system to offer this service a keystroke.  If *pIsEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten)
{
    _pThreadMgr->AddRef();
    *pIsEaten = key_event_sink_on_key_up(
        _pThreadMgr,
        _tfClientId,
        _pCompositionProcessorEngine->GetRaw(),
        _IsComposing(),
        _candidateMode,
        wParam,
        lParam
    );

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnPreservedKey
//
// Called when a hotkey (registered by us, or by the system) is typed.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pIsEaten)
{
	pContext;

    _pCompositionProcessorEngine->OnPreservedKey(rguid, pIsEaten, _GetThreadMgr(), _GetClientId());

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitKeyEventSink
//
// Advise a keystroke sink.
//----------------------------------------------------------------------------

BOOL CSampleIME::_InitKeyEventSink()
{
    _pThreadMgr->AddRef();
    this->AddRef();
    return key_event_sink_init_key_event_sink(_pThreadMgr, _tfClientId, (ITfKeyEventSink *)this);
}

//+---------------------------------------------------------------------------
//
// _UninitKeyEventSink
//
// Unadvise a keystroke sink.  Assumes we have advised one already.
//----------------------------------------------------------------------------

void CSampleIME::_UninitKeyEventSink()
{
    _pThreadMgr->AddRef();
    key_event_sink_uninit_key_event_sink(_pThreadMgr, _tfClientId);
}
