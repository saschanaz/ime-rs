// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "KeyStateCategory.h"

CKeyStateCategory* CKeyStateCategoryFactory::MakeKeyStateCategory(KeystrokeCategory keyCategory, _In_ CSampleIME *pTextService)
{
    CKeyStateCategory* pKeyState = nullptr;

    switch (keyCategory)
    {
    case KeystrokeCategory::Composing:
        pKeyState = new (std::nothrow) CKeyStateComposing(pTextService);
        break;

    case KeystrokeCategory::Candidate:
        pKeyState = new (std::nothrow) CKeyStateCandidate(pTextService);
        break;

    case KeystrokeCategory::None:
    default:
        pKeyState = new (std::nothrow) CKeyStateNull(pTextService);
        break;
    }
    return pKeyState;
}

/*
class CKeyStateCategory
*/
CKeyStateCategory::CKeyStateCategory(_In_ CSampleIME *pTextService)
{
    _pTextService = pTextService;
}

CKeyStateCategory::~CKeyStateCategory(void)
{
}

HRESULT CKeyStateCategory::KeyStateHandler(KeystrokeFunction function, KeyHandlerEditSessionDTO dto)
{
    switch(function)
    {
    case KeystrokeFunction::Input:
        return HandleKeyInput(dto);

    case KeystrokeFunction::FinalizeTextstoreAndInput:
        return HandleKeyFinalizeTextStoreAndInput(dto);

    case KeystrokeFunction::FinalizeTextstore:
        return HandleKeyFinalizeTextStore(dto);

    case KeystrokeFunction::FinalizeCandidatelistAndInput:
        return HandleKeyFinalizeCandidatelistAndInput(dto);

    case KeystrokeFunction::FinalizeCandidatelist:
        return HandleKeyFinalizeCandidatelist(dto);

    case KeystrokeFunction::Convert:
        return HandleKeyConvert(dto);

    case KeystrokeFunction::ConvertWildcard:
        return HandleKeyConvertWildCard(dto);

    case KeystrokeFunction::Cancel:
        return HandleKeyCancel(dto);

    case KeystrokeFunction::Backspace:
        return HandleKeyBackspace(dto);

    case KeystrokeFunction::MoveLeft:
    case KeystrokeFunction::MoveRight:
        return HandleKeyArrow(dto);

    case KeystrokeFunction::MoveUp:
    case KeystrokeFunction::MoveDown:
    case KeystrokeFunction::MovePageUp:
    case KeystrokeFunction::MovePageDown:
    case KeystrokeFunction::MovePageTop:
    case KeystrokeFunction::MovePageBottom:
        return HandleKeyArrow(dto);

    case KeystrokeFunction::DoubleSingleByte:
        return HandleKeyDoubleSingleByte(dto);

    case KeystrokeFunction::Punctuation:
        return HandleKeyPunctuation(dto);

    case KeystrokeFunction::SelectByNumber:
        return HandleKeySelectByNumber(dto);

    }
    return E_INVALIDARG;
}

void CKeyStateCategory::Release()
{
    delete this;
}

// _HandleCompositionInput
HRESULT CKeyStateCategory::HandleKeyInput(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

// HandleKeyFinalizeTextStore
HRESULT CKeyStateCategory::HandleKeyFinalizeTextStore(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}
// HandleKeyCompositionFinalizeTextStoreAndInput
HRESULT CKeyStateCategory::HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

// HandleKeyCompositionFinalizeCandidatelistAndInput
HRESULT CKeyStateCategory::HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

// HandleKeyCompositionFinalizeCandidatelist
HRESULT CKeyStateCategory::HandleKeyFinalizeCandidatelist(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

// HandleKeyConvert
HRESULT CKeyStateCategory::HandleKeyConvert(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

// HandleKeyConvertWildCard
HRESULT CKeyStateCategory::HandleKeyConvertWildCard(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

//_HandleCancel
HRESULT CKeyStateCategory::HandleKeyCancel(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

//_HandleCompositionBackspace
HRESULT CKeyStateCategory::HandleKeyBackspace(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

//_HandleCompositionArrowKey
HRESULT CKeyStateCategory::HandleKeyArrow(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

//_HandleCompositionDoubleSingleByte
HRESULT CKeyStateCategory::HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

//_HandleCompositionPunctuation
HRESULT CKeyStateCategory::HandleKeyPunctuation(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

HRESULT CKeyStateCategory::HandleKeySelectByNumber(KeyHandlerEditSessionDTO dto)
{
	dto;
    return E_NOTIMPL;
}

/*
class CKeyStateComposing
*/
CKeyStateComposing::CKeyStateComposing(_In_ CSampleIME *pTextService) : CKeyStateCategory(pTextService)
{
}

HRESULT CKeyStateComposing::HandleKeyInput(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionInput(dto.ec, dto.pContext, dto.wch);
}

HRESULT CKeyStateComposing::HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto)
{
    _pTextService->_HandleCompositionFinalize(dto.ec, dto.pContext, FALSE);
    return _pTextService->_HandleCompositionInput(dto.ec, dto.pContext, dto.wch);
}

HRESULT CKeyStateComposing::HandleKeyFinalizeTextStore(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionFinalize(dto.ec, dto.pContext, FALSE);
}

HRESULT CKeyStateComposing::HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto)
{
    _pTextService->_HandleCompositionFinalize(dto.ec, dto.pContext, TRUE);
    return _pTextService->_HandleCompositionInput(dto.ec, dto.pContext, dto.wch);
}

HRESULT CKeyStateComposing::HandleKeyFinalizeCandidatelist(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionFinalize(dto.ec, dto.pContext, TRUE);
}

HRESULT CKeyStateComposing::HandleKeyConvert(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionConvert(dto.ec, dto.pContext, FALSE);
}

HRESULT CKeyStateComposing::HandleKeyConvertWildCard(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionConvert(dto.ec, dto.pContext, TRUE);
}

HRESULT CKeyStateComposing::HandleKeyCancel(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCancel(dto.ec, dto.pContext);
}

HRESULT CKeyStateComposing::HandleKeyBackspace(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionBackspace(dto.ec, dto.pContext);
}

HRESULT CKeyStateComposing::HandleKeyArrow(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionArrowKey(dto.ec, dto.pContext, dto.arrowKey);
}

HRESULT CKeyStateComposing::HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionDoubleSingleByte(dto.ec, dto.pContext, dto.wch);
}

HRESULT CKeyStateComposing::HandleKeyPunctuation(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCompositionPunctuation(dto.ec, dto.pContext, dto.wch);
}

/*
class CKeyStateCandidate
*/
CKeyStateCandidate::CKeyStateCandidate(_In_ CSampleIME *pTextService) : CKeyStateCategory(pTextService)
{
}

// _HandleCandidateInput
HRESULT CKeyStateCandidate::HandleKeyFinalizeCandidatelist(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCandidateFinalize(dto.ec, dto.pContext);
}

// HandleKeyFinalizeCandidatelistAndInput
HRESULT CKeyStateCandidate::HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto)
{
    _pTextService->_HandleCandidateFinalize(dto.ec, dto.pContext);
    return _pTextService->_HandleCompositionInput(dto.ec, dto.pContext, dto.wch);
}

//_HandleCandidateConvert
HRESULT CKeyStateCandidate::HandleKeyConvert(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCandidateConvert(dto.ec, dto.pContext);
}

//_HandleCancel
HRESULT CKeyStateCandidate::HandleKeyCancel(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCancel(dto.ec, dto.pContext);
}

//_HandleCandidateArrowKey
HRESULT CKeyStateCandidate::HandleKeyArrow(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCandidateArrowKey(dto.ec, dto.pContext, dto.arrowKey);
}

//_HandleCandidateSelectByNumber
HRESULT CKeyStateCandidate::HandleKeySelectByNumber(KeyHandlerEditSessionDTO dto)
{
    return _pTextService->_HandleCandidateSelectByNumber(dto.ec, dto.pContext, dto.wch);
}
