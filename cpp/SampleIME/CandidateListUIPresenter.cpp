// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "SampleIME.h"
#include "CandidateWindow.h"
#include "CandidateListUIPresenter.h"
#include "CompositionProcessorEngine.h"
#include "SampleIMEBaseStructure.h"
#include "RustStringRange.h"
#include "cbindgen/globals.h"

//////////////////////////////////////////////////////////////////////
//
// CSampleIME candidate key handler methods
//
//////////////////////////////////////////////////////////////////////

const int MOVEUP_ONE = -1;
const int MOVEDOWN_ONE = 1;
const int MOVETO_TOP = 0;
const int MOVETO_BOTTOM = -1;
//+---------------------------------------------------------------------------
//
// _HandleCandidateFinalize
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCandidateFinalize(TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hr = S_OK;
    std::optional<CRustStringRange> candidateString;

    if (nullptr == _pCandidateListUIPresenter)
    {
        goto NoPresenter;
    }

    candidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();

    if (candidateString.has_value()) {
        hr = _AddComposingAndChar(ec, pContext, candidateString.value());

        if (FAILED(hr))
        {
            return hr;
        }
    }

NoPresenter:

    _HandleComplete(ec, pContext);

    return hr;
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateConvert
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCandidateConvert(TfEditCookie ec, _In_ ITfContext *pContext)
{
    return _HandleCandidateWorker(ec, pContext);
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateWorker
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCandidateWorker(TfEditCookie ec, _In_ ITfContext *pContext)
{
    HRESULT hrReturn = E_FAIL;
    DWORD_PTR candidateLen = 0;
    const WCHAR* pCandidateString = nullptr;
    std::optional<CRustStringRange> candidateString;
    CSampleImeArray<CCandidateListItem> candidatePhraseList;

    if (nullptr == _pCandidateListUIPresenter)
    {
        hrReturn = S_OK;
        goto Exit;
    }

    candidateString = _pCandidateListUIPresenter->_GetSelectedCandidateString();
    if (!candidateString.has_value()) {
        hrReturn = S_FALSE;
        goto Exit;
    }

    _pCompositionProcessorEngine->GetCandidateStringInConverted(candidateString.value(), &candidatePhraseList);

    _pCandidateListUIPresenter->RemoveSpecificCandidateFromList(candidatePhraseList, candidateString.value());

    // We have a candidate list if candidatePhraseList.Cnt is not 0
    // If we are showing reverse conversion, use CCandidateListUIPresenter
    CandidateMode tempCandMode = CandidateMode::None;
    CCandidateListUIPresenter* pTempCandListUIPresenter = nullptr;
    if (candidatePhraseList.Count())
    {
        tempCandMode = CandidateMode::WithNextComposition;

        pTempCandListUIPresenter = new (std::nothrow) CCandidateListUIPresenter(this, Global::AtomCandidateWindow,
            KeystrokeCategory::Candidate,
            FALSE);
        if (nullptr == pTempCandListUIPresenter)
        {
            hrReturn = E_OUTOFMEMORY;
            goto Exit;
        }
    }

    // call _Start*Line for CCandidateListUIPresenter or CReadingLine
    // we don't cache the document manager object so get it from pContext.
    ITfDocumentMgr* pDocumentMgr = nullptr;
    HRESULT hrStartCandidateList = E_FAIL;
    if (pContext->GetDocumentMgr(&pDocumentMgr) == S_OK)
    {
        ITfRange* pRange = nullptr;
        if (_pComposition->GetRange(&pRange) == S_OK)
        {
            if (pTempCandListUIPresenter)
            {
                hrStartCandidateList = pTempCandListUIPresenter->_StartCandidateList(_tfClientId, pDocumentMgr, pContext, ec, pRange, CAND_WIDTH);
            }

            pRange->Release();
        }
        pDocumentMgr->Release();
    }

    // set up candidate list if it is being shown
    if (SUCCEEDED(hrStartCandidateList))
    {
        pTempCandListUIPresenter->_SetTextColor(RGB(0, 0x80, 0), GetSysColor(COLOR_WINDOW));    // Text color is green
        pTempCandListUIPresenter->_SetFillColor((HBRUSH)(COLOR_WINDOW+1));    // Background color is window
        pTempCandListUIPresenter->_SetText(&candidatePhraseList);

        // Add composing character
        hrReturn = _AddComposingAndChar(ec, pContext, candidateString.value());

        // close candidate list
        if (_pCandidateListUIPresenter)
        {
            _pCandidateListUIPresenter->_EndCandidateList();
            delete _pCandidateListUIPresenter;
            _pCandidateListUIPresenter = nullptr;

            _candidateMode = CandidateMode::None;
        }

        if (hrReturn == S_OK)
        {
            // copy temp candidate
            _pCandidateListUIPresenter = pTempCandListUIPresenter;

            _candidateMode = tempCandMode;
        }
    }
    else
    {
        hrReturn = _HandleCandidateFinalize(ec, pContext);
    }

Exit:
    return hrReturn;
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateArrowKey
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCandidateArrowKey(TfEditCookie ec, _In_ ITfContext *pContext, _In_ KeystrokeFunction keyFunction)
{
    ec;
    pContext;

    _pCandidateListUIPresenter->AdviseUIChangedByArrowKey(keyFunction);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateSelectByNumber
//
//----------------------------------------------------------------------------

HRESULT CSampleIME::_HandleCandidateSelectByNumber(TfEditCookie ec, _In_ ITfContext *pContext, _In_ WCHAR wch)
{
    int iSelectAsNumber = CCandidateRange::GetIndex(wch);
    if (iSelectAsNumber == -1)
    {
        return S_FALSE;
    }

    if (_pCandidateListUIPresenter)
    {
        if (_pCandidateListUIPresenter->_SetSelectionInPage(iSelectAsNumber))
        {
            return _HandleCandidateConvert(ec, pContext);
        }
    }

    return S_FALSE;
}

//////////////////////////////////////////////////////////////////////
//
// CCandidateListUIPresenter class
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CCandidateListUIPresenter::CCandidateListUIPresenter(_In_ CSampleIME *pTextService, ATOM atom, KeystrokeCategory Category, BOOL hideWindow) : CTfTextLayoutSink(pTextService)
{
    _atom = atom;

    _parentWndHandle = nullptr;
    _pCandidateWnd = nullptr;

    _Category = Category;

    _updatedFlags = 0;

    _uiElementId = (DWORD)-1;
    _isShowMode = TRUE;   // store return value from BeginUIElement
    _hideWindow = hideWindow;     // Hide window flag from [Configuration] CandidateList.Phrase.HideWindow

    _pTextService = pTextService;
    _pTextService->AddRef();

    _refCount = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CCandidateListUIPresenter::~CCandidateListUIPresenter()
{
    _EndCandidateList();
    _pTextService->Release();
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::IUnknown::QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
{
    if (CTfTextLayoutSink::QueryInterface(riid, ppvObj) == S_OK)
    {
        return S_OK;
    }

    if (ppvObj == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_ITfUIElement) ||
        IsEqualIID(riid, IID_ITfCandidateListUIElement))
    {
        *ppvObj = (ITfCandidateListUIElement*)this;
    }
    else if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfCandidateListUIElementBehavior))
    {
        *ppvObj = (ITfCandidateListUIElementBehavior*)this;
    }
    else if (IsEqualIID(riid, __uuidof(ITfIntegratableCandidateListUIElement)))
    {
        *ppvObj = (ITfIntegratableCandidateListUIElement*)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::IUnknown::AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCandidateListUIPresenter::AddRef()
{
    CTfTextLayoutSink::AddRef();
    return ++_refCount;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::IUnknown::Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCandidateListUIPresenter::Release()
{
    CTfTextLayoutSink::Release();

    LONG cr = --_refCount;

    assert(_refCount >= 0);

    if (_refCount == 0)
    {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::ITfUIElement::GetDescription
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetDescription(BSTR *pbstr)
{
    if (pbstr)
    {
        *pbstr = SysAllocString(L"Cand");
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::ITfUIElement::GetGUID
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetGUID(GUID *pguid)
{
    *pguid = SAMPLEIME_GUID_CAND_UIELEMENT;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::ITfUIElement::Show
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::Show(BOOL showCandidateWindow)
{
    if (showCandidateWindow)
    {
        ToShowCandidateWindow();
    }
    else
    {
        ToHideCandidateWindow();
    }
    return S_OK;
}

HRESULT CCandidateListUIPresenter::ToShowCandidateWindow()
{
    if (_hideWindow)
    {
        _pCandidateWnd->_Show(FALSE);
    }
    else
    {
        _MoveWindowToTextExt();

        _pCandidateWnd->_Show(TRUE);
    }

    return S_OK;
}

HRESULT CCandidateListUIPresenter::ToHideCandidateWindow()
{
	if (_pCandidateWnd)
	{
		_pCandidateWnd->_Show(FALSE);
	}

    _updatedFlags = TF_CLUIE_SELECTION | TF_CLUIE_CURRENTPAGE;
    _UpdateUIElement();

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::ITfUIElement::IsShown
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::IsShown(BOOL *pIsShow)
{
    *pIsShow = _pCandidateWnd->_IsWindowVisible();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetUpdatedFlags
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetUpdatedFlags(DWORD *pdwFlags)
{
    *pdwFlags = _updatedFlags;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetDocumentMgr
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetDocumentMgr(ITfDocumentMgr **ppdim)
{
    *ppdim = nullptr;

    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetCount
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetCount(UINT *pCandidateCount)
{
    if (_pCandidateWnd)
    {
        *pCandidateCount = _pCandidateWnd->_GetCount();
    }
    else
    {
        *pCandidateCount = 0;
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetSelection
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetSelection(UINT *pSelectedCandidateIndex)
{
    if (_pCandidateWnd)
    {
        *pSelectedCandidateIndex = _pCandidateWnd->_GetSelection();
    }
    else
    {
        *pSelectedCandidateIndex = 0;
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetString
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetString(UINT uIndex, BSTR *pbstr)
{
    if (!_pCandidateWnd || (uIndex > _pCandidateWnd->_GetCount()))
    {
        return E_FAIL;
    }

    std::optional<CRustStringRange> candidateString = _pCandidateWnd->_GetCandidateString(uIndex);

    if (candidateString.has_value()) {
        CStringRangeUtf16 str(candidateString.value());
        *pbstr = SysAllocStringLen(str.GetRaw(), (DWORD)str.GetLength());
    } else {
        *pbstr = nullptr;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetPageIndex
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt)
{
    if (!_pCandidateWnd)
    {
        if (pIndex)
        {
            *pIndex = 0;
        }
        *puPageCnt = 0;
        return S_OK;
    }
    return _pCandidateWnd->_GetPageIndex(pIndex, uSize, puPageCnt);
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::SetPageIndex
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::SetPageIndex(UINT *pIndex, UINT uPageCnt)
{
    if (!_pCandidateWnd)
    {
        return E_FAIL;
    }
    return _pCandidateWnd->_SetPageIndex(pIndex, uPageCnt);
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement::GetCurrentPage
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetCurrentPage(UINT *puPage)
{
    if (!_pCandidateWnd)
    {
        *puPage = 0;
        return S_OK;
    }
    return _pCandidateWnd->_GetCurrentPage(puPage);
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElementBehavior::SetSelection
// It is related of the mouse clicking behavior upon the suggestion window
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::SetSelection(UINT nIndex)
{
    if (_pCandidateWnd)
    {
        _pCandidateWnd->_SetSelection(nIndex);
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElementBehavior::Finalize
// It is related of the mouse clicking behavior upon the suggestion window
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::Finalize(void)
{
    _CandidateChangeNotification(CAND_ITEM_SELECT);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElementBehavior::Abort
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::Abort(void)
{
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfIntegratableCandidateListUIElement::SetIntegrationStyle
// To show candidateNumbers on the suggestion window
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::SetIntegrationStyle(GUID guidIntegrationStyle)
{
    return (guidIntegrationStyle == GUID_INTEGRATIONSTYLE_SEARCHBOX) ? S_OK : E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfIntegratableCandidateListUIElement::GetSelectionStyle
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::GetSelectionStyle(_Out_ TfIntegratableCandidateListSelectionStyle *ptfSelectionStyle)
{
    *ptfSelectionStyle = STYLE_ACTIVE_SELECTION;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfIntegratableCandidateListUIElement::OnKeyDown
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::OnKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL *pIsEaten)
{
    wParam;
    lParam;

    *pIsEaten = TRUE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfIntegratableCandidateListUIElement::ShowCandidateNumbers
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::ShowCandidateNumbers(_Out_ BOOL *pIsShow)
{
    *pIsShow = TRUE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfIntegratableCandidateListUIElement::FinalizeExactCompositionString
//
//----------------------------------------------------------------------------

STDAPI CCandidateListUIPresenter::FinalizeExactCompositionString()
{
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
// _StartCandidateList
//
//----------------------------------------------------------------------------

HRESULT CCandidateListUIPresenter::_StartCandidateList(TfClientId tfClientId, _In_ ITfDocumentMgr *pDocumentMgr, _In_ ITfContext *pContextDocument, TfEditCookie ec, _In_ ITfRange *pRangeComposition, UINT wndWidth)
{
	pDocumentMgr;tfClientId;

    HRESULT hr = E_FAIL;

    if (FAILED(_StartLayout(pContextDocument, ec, pRangeComposition)))
    {
        goto Exit;
    }

    BeginUIElement();

    hr = MakeCandidateWindow(pContextDocument, wndWidth);
    if (FAILED(hr))
    {
        goto Exit;
    }

    Show(_isShowMode);

    RECT rcTextExt;
    if (SUCCEEDED(_GetTextExt(&rcTextExt)))
    {
        _LayoutChangeNotification(&rcTextExt);
    }

Exit:
    if (FAILED(hr))
    {
        _EndCandidateList();
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
// _EndCandidateList
//
//----------------------------------------------------------------------------

void CCandidateListUIPresenter::_EndCandidateList()
{
    EndUIElement();

    DisposeCandidateWindow();

    _EndLayout();
}

//+---------------------------------------------------------------------------
//
// _SetText
//
//----------------------------------------------------------------------------

void CCandidateListUIPresenter::_SetText(_In_ CSampleImeArray<CCandidateListItem> *pCandidateList)
{
    AddCandidateToCandidateListUI(pCandidateList);

    SetPageIndexWithScrollInfo(pCandidateList);

    if (_isShowMode)
    {
        _pCandidateWnd->_InvalidateRect();
    }
    else
    {
        _updatedFlags = TF_CLUIE_COUNT       |
            TF_CLUIE_SELECTION   |
            TF_CLUIE_STRING      |
            TF_CLUIE_PAGEINDEX   |
            TF_CLUIE_CURRENTPAGE;
        _UpdateUIElement();
    }
}

void CCandidateListUIPresenter::AddCandidateToCandidateListUI(_In_ CSampleImeArray<CCandidateListItem> *pCandidateList)
{
    for (auto& item : *pCandidateList)
    {
        _pCandidateWnd->_AddString(CRustStringRange(item._ItemString));
    }
}

void CCandidateListUIPresenter::SetPageIndexWithScrollInfo(_In_ CSampleImeArray<CCandidateListItem> *pCandidateList)
{
    UINT candCntInPage = CCandidateRange::Count;
    UINT bufferSize = pCandidateList->Count() / candCntInPage + 1;
    UINT* puPageIndex = new (std::nothrow) UINT[ bufferSize ];
    if (puPageIndex != nullptr)
    {
        for (UINT i = 0; i < bufferSize; i++)
        {
            puPageIndex[i] = i * candCntInPage;
        }

        _pCandidateWnd->_SetPageIndex(puPageIndex, bufferSize);
        delete [] puPageIndex;
    }
    _pCandidateWnd->_SetScrollInfo(pCandidateList->Count(), candCntInPage);  // nMax:range of max, nPage:number of items in page

}
//+---------------------------------------------------------------------------
//
// _ClearList
//
//----------------------------------------------------------------------------

void CCandidateListUIPresenter::_ClearList()
{
    _pCandidateWnd->_ClearList();
    _pCandidateWnd->_InvalidateRect();
}

//+---------------------------------------------------------------------------
//
// _SetTextColor
// _SetFillColor
//
//----------------------------------------------------------------------------

void CCandidateListUIPresenter::_SetTextColor(COLORREF crColor, COLORREF crBkColor)
{
    _pCandidateWnd->_SetTextColor(crColor, crBkColor);
}

void CCandidateListUIPresenter::_SetFillColor(HBRUSH hBrush)
{
    _pCandidateWnd->_SetFillColor(hBrush);
}

//+---------------------------------------------------------------------------
//
// _GetSelectedCandidateString
//
//----------------------------------------------------------------------------

std::optional<CRustStringRange> CCandidateListUIPresenter::_GetSelectedCandidateString()
{
    return _pCandidateWnd->_GetSelectedCandidateString();
}

//+---------------------------------------------------------------------------
//
// _MoveSelection
//
//----------------------------------------------------------------------------

BOOL CCandidateListUIPresenter::_MoveSelection(_In_ int offSet)
{
    BOOL ret = _pCandidateWnd->_MoveSelection(offSet, TRUE);
    if (ret)
    {
        if (_isShowMode)
        {
            _pCandidateWnd->_InvalidateRect();
        }
        else
        {
            _updatedFlags = TF_CLUIE_SELECTION;
            _UpdateUIElement();
        }
    }
    return ret;
}

//+---------------------------------------------------------------------------
//
// _SetSelection
//
//----------------------------------------------------------------------------

BOOL CCandidateListUIPresenter::_SetSelection(_In_ int selectedIndex)
{
    BOOL ret = _pCandidateWnd->_SetSelection(selectedIndex, TRUE);
    if (ret)
    {
        if (_isShowMode)
        {
            _pCandidateWnd->_InvalidateRect();
        }
        else
        {
            _updatedFlags = TF_CLUIE_SELECTION |
                TF_CLUIE_CURRENTPAGE;
            _UpdateUIElement();
        }
    }
    return ret;
}

//+---------------------------------------------------------------------------
//
// _MovePage
//
//----------------------------------------------------------------------------

BOOL CCandidateListUIPresenter::_MovePage(_In_ int offSet)
{
    BOOL ret = _pCandidateWnd->_MovePage(offSet, TRUE);
    if (ret)
    {
        if (_isShowMode)
        {
            _pCandidateWnd->_InvalidateRect();
        }
        else
        {
            _updatedFlags = TF_CLUIE_SELECTION |
                TF_CLUIE_CURRENTPAGE;
            _UpdateUIElement();
        }
    }
    return ret;
}

//+---------------------------------------------------------------------------
//
// _MoveWindowToTextExt
//
//----------------------------------------------------------------------------

void CCandidateListUIPresenter::_MoveWindowToTextExt()
{
    RECT rc;

    if (FAILED(_GetTextExt(&rc)))
    {
        return;
    }

    _pCandidateWnd->_Move(rc.left, rc.bottom);
}
//+---------------------------------------------------------------------------
//
// _LayoutChangeNotification
//
//----------------------------------------------------------------------------

VOID CCandidateListUIPresenter::_LayoutChangeNotification(_In_ RECT *lpRect)
{
    RECT rectCandidate = {0, 0, 0, 0};
    POINT ptCandidate = {0, 0};

    _pCandidateWnd->_GetClientRect(&rectCandidate);
    _pCandidateWnd->_GetWindowExtent(lpRect, &rectCandidate, &ptCandidate);
    _pCandidateWnd->_Move(ptCandidate.x, ptCandidate.y);
}

//+---------------------------------------------------------------------------
//
// _LayoutDestroyNotification
//
//----------------------------------------------------------------------------

VOID CCandidateListUIPresenter::_LayoutDestroyNotification()
{
    _EndCandidateList();
}

//+---------------------------------------------------------------------------
//
// _CandidateChangeNotifiction
//
//----------------------------------------------------------------------------

HRESULT CCandidateListUIPresenter::_CandidateChangeNotification(_In_ enum CANDWND_ACTION action)
{
    HRESULT hr = E_FAIL;

    TfClientId tfClientId = _pTextService->_GetClientId();
    ITfThreadMgr* pThreadMgr = nullptr;
    ITfDocumentMgr* pDocumentMgr = nullptr;
    ITfContext* pContext = nullptr;

    _KEYSTROKE_STATE KeyState;
    KeyState.Category = _Category;
    KeyState.Function = KeystrokeFunction::FinalizeCandidatelist;

    if (CAND_ITEM_SELECT != action)
    {
        goto Exit;
    }

    pThreadMgr = _pTextService->_GetThreadMgr();
    if (nullptr == pThreadMgr)
    {
        goto Exit;
    }

    hr = pThreadMgr->GetFocus(&pDocumentMgr);
    if (FAILED(hr))
    {
        goto Exit;
    }

    hr = pDocumentMgr->GetTop(&pContext);
    if (FAILED(hr))
    {
        pDocumentMgr->Release();
        goto Exit;
    }

    CKeyHandlerEditSession *pEditSession = new (std::nothrow) CKeyHandlerEditSession(_pTextService, pContext, 0, KeyState);
    if (nullptr != pEditSession)
    {
        HRESULT hrSession = S_OK;
        hr = pContext->RequestEditSession(tfClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hrSession);
        if (hrSession == TF_E_SYNCHRONOUS || hrSession == TS_E_READONLY)
        {
            hr = pContext->RequestEditSession(tfClientId, pEditSession, TF_ES_ASYNC | TF_ES_READWRITE, &hrSession);
        }
        pEditSession->Release();
    }

    pContext->Release();
    pDocumentMgr->Release();

Exit:
    return hr;
}

//+---------------------------------------------------------------------------
//
// _CandWndCallback
//
//----------------------------------------------------------------------------

// static
HRESULT CCandidateListUIPresenter::_CandWndCallback(_In_ void *pv, _In_ enum CANDWND_ACTION action)
{
    CCandidateListUIPresenter* fakeThis = (CCandidateListUIPresenter*)pv;

    return fakeThis->_CandidateChangeNotification(action);
}

//+---------------------------------------------------------------------------
//
// _UpdateUIElement
//
//----------------------------------------------------------------------------

HRESULT CCandidateListUIPresenter::_UpdateUIElement()
{
    HRESULT hr = S_OK;

    ITfThreadMgr* pThreadMgr = _pTextService->_GetThreadMgr();
    if (nullptr == pThreadMgr)
    {
        return S_OK;
    }

    ITfUIElementMgr* pUIElementMgr = nullptr;

    hr = pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr);
    if (hr == S_OK)
    {
        pUIElementMgr->UpdateUIElement(_uiElementId);
        pUIElementMgr->Release();
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnSetThreadFocus
//
//----------------------------------------------------------------------------

HRESULT CCandidateListUIPresenter::OnSetThreadFocus()
{
    if (_isShowMode)
    {
        Show(TRUE);
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnKillThreadFocus
//
//----------------------------------------------------------------------------

HRESULT CCandidateListUIPresenter::OnKillThreadFocus()
{
    if (_isShowMode)
    {
        Show(FALSE);
    }
    return S_OK;
}

void CCandidateListUIPresenter::RemoveSpecificCandidateFromList(_Inout_ CSampleImeArray<CCandidateListItem> &candidateList, const CRustStringRange& candidateString)
{
    for (UINT index = 0; index < candidateList.Count();)
    {
        CCandidateListItem* pLI = candidateList.GetAt(index);

        if (candidateString == CRustStringRange(pLI->_ItemString))
        {
            candidateList.RemoveAt(index);
            continue;
        }

        index++;
    }
}

void CCandidateListUIPresenter::AdviseUIChangedByArrowKey(_In_ KeystrokeFunction arrowKey)
{
    switch (arrowKey)
    {
    case KeystrokeFunction::MoveUp:
        {
            _MoveSelection(MOVEUP_ONE);
            break;
        }
    case KeystrokeFunction::MoveDown:
        {
            _MoveSelection(MOVEDOWN_ONE);
            break;
        }
    case KeystrokeFunction::MovePageUp:
        {
            _MovePage(MOVEUP_ONE);
            break;
        }
    case KeystrokeFunction::MovePageDown:
        {
            _MovePage(MOVEDOWN_ONE);
            break;
        }
    case KeystrokeFunction::MovePageTop:
        {
            _SetSelection(MOVETO_TOP);
            break;
        }
    case KeystrokeFunction::MovePageBottom:
        {
            _SetSelection(MOVETO_BOTTOM);
            break;
        }
    default:
        break;
    }
}

HRESULT CCandidateListUIPresenter::BeginUIElement()
{
    HRESULT hr = S_OK;

    ITfThreadMgr* pThreadMgr = _pTextService->_GetThreadMgr();
    if (nullptr ==pThreadMgr)
    {
        hr = E_FAIL;
        goto Exit;
    }

    ITfUIElementMgr* pUIElementMgr = nullptr;
    hr = pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr);
    if (hr == S_OK)
    {
        pUIElementMgr->BeginUIElement(this, &_isShowMode, &_uiElementId);
        pUIElementMgr->Release();
    }

Exit:
    return hr;
}

HRESULT CCandidateListUIPresenter::EndUIElement()
{
    HRESULT hr = S_OK;

    ITfThreadMgr* pThreadMgr = _pTextService->_GetThreadMgr();
    if ((nullptr == pThreadMgr) || (-1 == _uiElementId))
    {
        hr = E_FAIL;
        goto Exit;
    }

    ITfUIElementMgr* pUIElementMgr = nullptr;
    hr = pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr);
    if (hr == S_OK)
    {
        pUIElementMgr->EndUIElement(_uiElementId);
        pUIElementMgr->Release();
    }

Exit:
    return hr;
}

HRESULT CCandidateListUIPresenter::MakeCandidateWindow(_In_ ITfContext *pContextDocument, _In_ UINT wndWidth)
{
    HRESULT hr = S_OK;

    if (nullptr != _pCandidateWnd)
    {
        return hr;
    }

    _pCandidateWnd = new (std::nothrow) CCandidateWindow(_CandWndCallback, this, _pTextService->_IsStoreAppMode());
    if (_pCandidateWnd == nullptr)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    HWND parentWndHandle = nullptr;
    ITfContextView* pView = nullptr;
    if (SUCCEEDED(pContextDocument->GetActiveView(&pView)))
    {
        pView->GetWnd(&parentWndHandle);
    }

    if (!_pCandidateWnd->_Create(_atom, wndWidth, parentWndHandle))
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

Exit:
    return hr;
}

void CCandidateListUIPresenter::DisposeCandidateWindow()
{
    if (nullptr == _pCandidateWnd)
    {
        return;
    }

    _pCandidateWnd->_Destroy();

    delete _pCandidateWnd;
    _pCandidateWnd = nullptr;
}
