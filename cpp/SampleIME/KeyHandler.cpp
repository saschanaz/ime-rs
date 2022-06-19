// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "EditSession.h"
#include "SampleIME.h"
#include "CandidateListUIPresenter.h"
#include "CompositionProcessorEngine.h"

//////////////////////////////////////////////////////////////////////
//
// CSampleIME class
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// _IsRangeCovered
//
// Returns TRUE if pRangeTest is entirely contained within pRangeCover.
//
//----------------------------------------------------------------------------

BOOL CSampleIME::_IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover)
{
    LONG lResult = 0;;

    if (FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult))
        || (lResult > 0))
    {
        return FALSE;
    }

    if (FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult))
        || (lResult < 0))
    {
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _DeleteCandidateList
//
//----------------------------------------------------------------------------

VOID CSampleIME::_DeleteCandidateList(BOOL isForce, _In_opt_ ITfContext *pContext)
{
    isForce;pContext;

    CCompositionProcessorEngine* pCompositionProcessorEngine = nullptr;
    pCompositionProcessorEngine = _pCompositionProcessorEngine;
    pCompositionProcessorEngine->PurgeVirtualKey();

    if (_pCandidateListUIPresenter)
    {
        _pCandidateListUIPresenter->_EndCandidateList();

        _candidateMode = CandidateMode::None;
    }
}

//+---------------------------------------------------------------------------
//
// _HandleComplete
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleComplete(TfEditCookie ec, _In_ ITfContext *pContext)
{
    _DeleteCandidateList(FALSE, pContext);

    // just terminate the composition
    _TerminateComposition(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCancel
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCancel(TfEditCookie ec, _In_ ITfContext *pContext)
{
    _RemoveDummyCompositionForComposing(ec, _pComposition);

    _DeleteCandidateList(FALSE, pContext);

    _TerminateComposition(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionInput
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionInput(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;
    BOOL isCovered = TRUE;

    CCompositionProcessorEngine* pCompositionProcessorEngine = nullptr;
    pCompositionProcessorEngine = _pCompositionProcessorEngine;

    if ((_pCandidateListUIPresenter != nullptr) && (_candidateMode != CandidateMode::Incremental))
    {
        _HandleCompositionFinalize(ec, pContext, FALSE);
    }

    // Start the new (std::nothrow) compositon if there is no composition.
    if (!_IsComposing())
    {
        _StartComposition(pContext);
    }

    // first, test where a keystroke would go in the document if we did an insert
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched) != S_OK || fetched != 1)
    {
        return S_FALSE;
    }

    // is the insertion point covered by a composition?
    if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
    {
        isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!isCovered)
        {
            goto Exit;
        }
    }

    // Add virtual key to composition processor engine
    pCompositionProcessorEngine->AddVirtualKey(wch);

    _HandleCompositionInputWorker(pCompositionProcessorEngine, ec, pContext);

Exit:
    tfSelection.range->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionInputWorker
//
// If the keystroke happens within a composition, eat the key and return S_OK.
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionInputWorker(_In_ CCompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;
    BOOL isWildcardIncluded = TRUE;

    //
    // Get reading string from composition processor engine
    //
    bool hasVirtualKey = pCompositionProcessorEngine->HasVirtualKey();

    if (hasVirtualKey)
    {
        CRustStringRange item = pCompositionProcessorEngine->KeystrokeBufferGetReadingString();
        isWildcardIncluded = pCompositionProcessorEngine->KeystrokeBufferIncludesWildcard();

        hr = _AddComposingAndChar(ec, pContext, item);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    //
    // Get candidate string from composition processor engine
    //
    CSampleImeArray<CCandidateListItem> candidateList;

    pCompositionProcessorEngine->GetCandidateList(&candidateList, TRUE, FALSE);

    if ((candidateList.Count()))
    {
        hr = _CreateAndStartCandidate(pCompositionProcessorEngine, ec, pContext);
        if (SUCCEEDED(hr))
        {
            _pCandidateListUIPresenter->_ClearList();
            _pCandidateListUIPresenter->_SetText(&candidateList);
        }
    }
    else if (_pCandidateListUIPresenter)
    {
        _pCandidateListUIPresenter->_ClearList();
    }
    else if (hasVirtualKey && isWildcardIncluded)
    {
        hr = _CreateAndStartCandidate(pCompositionProcessorEngine, ec, pContext);
        if (SUCCEEDED(hr))
        {
            _pCandidateListUIPresenter->_ClearList();
        }
    }
    return hr;
}
//+---------------------------------------------------------------------------
//
// _CreateAndStartCandidate
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_CreateAndStartCandidate(_In_ CCompositionProcessorEngine *pCompositionProcessorEngine, TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;

    if ((_candidateMode == CandidateMode::None) && (_pCandidateListUIPresenter))
    {
        // Recreate candidate list
        _pCandidateListUIPresenter->_EndCandidateList();
        delete _pCandidateListUIPresenter;
        _pCandidateListUIPresenter = nullptr;

        _candidateMode = CandidateMode::None;
    }

    if (_pCandidateListUIPresenter == nullptr)
    {
        _pCandidateListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(this, Global::AtomCandidateWindow,
            KeystrokeCategory::Candidate,
            FALSE);
        if (!_pCandidateListUIPresenter)
        {
            return E_OUTOFMEMORY;
        }

        _candidateMode = CandidateMode::Incremental;

        // we don't cache the document manager object. So get it from pContext.
        ITfDocumentMgr* pDocumentMgr = nullptr;
        if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
        {
            // get the composition range.
            ITfRange* pRange = nullptr;
            if (SUCCEEDED(_pComposition->GetRange(&pRange)))
            {
                hr = _pCandidateListUIPresenter->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange, CAND_WIDTH);
                pRange->Release();
            }
            pDocumentMgr->Release();
        }
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionFinalize
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionFinalize(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCandidateList)
{
    HRESULT hr = S_OK;

    if (isCandidateList && _pCandidateListUIPresenter)
    {
        // Finalize selected candidate string from CCandidateListUIPresenter
        std::optional<CRustStringRange> candidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();

        if (candidateString.has_value())
        {
            // Finalize character
            hr = _AddCharAndFinalize(ec, pContext, candidateString.value());
            if (FAILED(hr))
            {
                return hr;
            }
        }
    }
    else
    {
        // Finalize current text store strings
        if (_IsComposing())
        {
            ULONG fetched = 0;
            TF_SELECTION tfSelection;

            if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
            {
                return S_FALSE;
            }

            ITfRange* pRangeComposition = nullptr;
            if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
            {
                if (_IsRangeCovered(ec, tfSelection.range, pRangeComposition))
                {
                    _EndComposition(pContext);
                }

                pRangeComposition->Release();
            }

            tfSelection.range->Release();
        }
    }

    _HandleCancel(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionConvert
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionConvert(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isWildcardSearch)
{
    HRESULT hr = S_OK;

    CSampleImeArray<CCandidateListItem> candidateList;

    //
    // Get candidate string from composition processor engine
    //
    CCompositionProcessorEngine* pCompositionProcessorEngine = nullptr;
    pCompositionProcessorEngine = _pCompositionProcessorEngine;
    pCompositionProcessorEngine->GetCandidateList(&candidateList, FALSE, isWildcardSearch);

    // If there is no candlidate listing the current reading string, we don't do anything. Just wait for
    // next char to be ready for the conversion with it.
    int nCount = candidateList.Count();
    if (nCount)
    {
        if (_pCandidateListUIPresenter)
        {
            _pCandidateListUIPresenter->_EndCandidateList();
            delete _pCandidateListUIPresenter;
            _pCandidateListUIPresenter = nullptr;

            _candidateMode = CandidateMode::None;
        }

        //
        // create an instance of the candidate list class.
        //
        _pCandidateListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(this, Global::AtomCandidateWindow,
            KeystrokeCategory::Candidate,
            FALSE);
        if (!_pCandidateListUIPresenter)
        {
            return E_OUTOFMEMORY;
        }

        _candidateMode = CandidateMode::Original;

        // we don't cache the document manager object. So get it from pContext.
        ITfDocumentMgr* pDocumentMgr = nullptr;
        if (SUCCEEDED(pContext->GetDocumentMgr(&pDocumentMgr)))
        {
            // get the composition range.
            ITfRange* pRange = nullptr;
            if (SUCCEEDED(_pComposition->GetRange(&pRange)))
            {
                hr = _pCandidateListUIPresenter->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange, CAND_WIDTH);
                pRange->Release();
            }
            pDocumentMgr->Release();
        }
        if (SUCCEEDED(hr))
        {
            _pCandidateListUIPresenter->_SetText(&candidateList);
        }
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionBackspace
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionBackspace(TfEditCookie ec, _In_ ITfContext *pContext)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;
    BOOL isCovered = TRUE;

    // Start the new (std::nothrow) compositon if there is no composition.
    if (!_IsComposing())
    {
        return S_OK;
    }

    // first, test where a keystroke would go in the document if we did an insert
    if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched)) || fetched != 1)
    {
        return S_FALSE;
    }

    // is the insertion point covered by a composition?
    if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition)))
    {
        isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!isCovered)
        {
            goto Exit;
        }
    }

    //
    // Add virtual key to composition processor engine
    //
    CCompositionProcessorEngine* pCompositionProcessorEngine = nullptr;
    pCompositionProcessorEngine = _pCompositionProcessorEngine;

    if (pCompositionProcessorEngine->HasVirtualKey())
    {
        pCompositionProcessorEngine->PopVirtualKey();

        if (pCompositionProcessorEngine->HasVirtualKey())
        {
            _HandleCompositionInputWorker(pCompositionProcessorEngine, ec, pContext);
        }
        else
        {
            _HandleCancel(ec, pContext);
        }
    }

Exit:
    tfSelection.range->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionArrowKey
//
// Update the selection within a composition.
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, KeystrokeFunction keyFunction)
{
    ITfRange* pRangeComposition = nullptr;
    TF_SELECTION tfSelection;
    ULONG fetched = 0;

    // get the selection
    if (FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched))
        || fetched != 1)
    {
        // no selection, eat the keystroke
        return S_OK;
    }

    // get the composition range
    if (FAILED(_pComposition->GetRange(&pRangeComposition)))
    {
        goto Exit;
    }

    // For incremental candidate list
    if (_pCandidateListUIPresenter)
    {
        _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(keyFunction);
    }

    pContext->SetSelection(ec, 1, &tfSelection);

    pRangeComposition->Release();

Exit:
    tfSelection.range->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionPunctuation
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionPunctuation(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    HRESULT hr = S_OK;

    if (_candidateMode != CandidateMode::None && _pCandidateListUIPresenter)
    {
        std::optional<CRustStringRange> candidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();

        if (candidateString.has_value())
        {
            _AddComposingAndChar(ec, pContext, candidateString.value());
        }
    }
    //
    // Get punctuation char from composition processor engine
    //
    CCompositionProcessorEngine* pCompositionProcessorEngine = nullptr;
    pCompositionProcessorEngine = _pCompositionProcessorEngine;

    WCHAR punctuation = pCompositionProcessorEngine->PunctuationsGetAlternativePunctuationCounted(wch);

    // Finalize character
    hr = _AddCharAndFinalize(ec, pContext, CRustStringRange(CStringRangeUtf16(punctuation)));
    if (FAILED(hr))
    {
        return hr;
    }

    _HandleCancel(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCompositionDoubleSingleByte
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCompositionDoubleSingleByte(TfEditCookie ec, _In_ ITfContext *pContext, WCHAR wch)
{
    HRESULT hr = S_OK;

    // \u3000 or Unicode "Fullwidth ASCII variants" (U+FF01 to U+FF5E)
    // https://www.unicode.org/charts/PDF/UFF00.pdf
    WCHAR fullWidth = wch == 0x20 ? u'\u3000' : (wch - 0x20 + 0xff00);

    // Finalize character
    hr = _AddCharAndFinalize(ec, pContext, CRustStringRange(CStringRangeUtf16(fullWidth)));
    if (FAILED(hr))
    {
        return hr;
    }

    _HandleCancel(ec, pContext);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InvokeKeyHandler
//
// This text service is interested in handling keystrokes to demonstrate the
// use the compositions. Some apps will cancel compositions if they receive
// keystrokes while a compositions is ongoing.
//
// param
//    [in] uCode - virtual key code of WM_KEYDOWN wParam
//    [in] dwFlags - WM_KEYDOWN lParam
//    [in] dwKeyFunction - Function regarding virtual key
//----------------------------------------------------------------------------

HRESULT CSampleIME::_InvokeKeyHandler(_In_ ITfContext *pContext, WCHAR wch, DWORD flags, _KEYSTROKE_STATE keyState)
{
    flags;

    CKeyHandlerEditSession* pEditSession = nullptr;
    HRESULT hr = E_FAIL;

    // we'll insert a char ourselves in place of this keystroke
    pEditSession = new (std::nothrow) CKeyHandlerEditSession(this, pContext, wch, keyState);
    if (pEditSession == nullptr)
    {
        goto Exit;
    }

    //
    // Call CKeyHandlerEditSession::DoEditSession().
    //
    // Do not specify TF_ES_SYNC so edit session is not invoked on WinWord
    //
    hr = pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);

    pEditSession->Release();

Exit:
    return hr;
}
