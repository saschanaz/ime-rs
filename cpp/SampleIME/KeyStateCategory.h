// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "Globals.h"
#include "Private.h"
#include "SampleIME.h"

class CKeyStateCategory;

class CKeyStateCategoryFactory
{
public:
    static CKeyStateCategory* MakeKeyStateCategory(KeystrokeCategory keyCategory, _In_ CSampleIME *pTextService);
};

typedef struct KeyHandlerEditSessionDTO
{
    KeyHandlerEditSessionDTO::KeyHandlerEditSessionDTO(TfEditCookie tFEC, _In_ ITfContext *pTfContext, WCHAR inputChar, KeystrokeFunction arrowKeyFunction)
    {
        ec = tFEC;
        pContext = pTfContext;
        wch = inputChar;
        arrowKey = arrowKeyFunction;
    }

    TfEditCookie ec;
    ITfContext* pContext;
    WCHAR wch;
    KeystrokeFunction arrowKey;
}KeyHandlerEditSessionDTO;

class CKeyStateCategory
{
public:
    CKeyStateCategory(_In_ CSampleIME *pTextService);

protected:
    ~CKeyStateCategory(void);

public:
    HRESULT KeyStateHandler(KeystrokeFunction function, KeyHandlerEditSessionDTO dto);
    void Release(void);

protected:
    // HandleKeyInput
    virtual HRESULT HandleKeyInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyFinalizeTextStoreAndInput
    virtual HRESULT HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyFinalizeTextStore
    virtual HRESULT HandleKeyFinalizeTextStore(KeyHandlerEditSessionDTO dto);

    // HandleKeyFinalizeCandidatelistAndInput
    virtual HRESULT HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyFinalizeCandidatelist
    virtual HRESULT HandleKeyFinalizeCandidatelist(KeyHandlerEditSessionDTO dto);

    // HandleKeyConvert
    virtual HRESULT HandleKeyConvert(KeyHandlerEditSessionDTO dto);

    // HandleKeyConvertWild
    virtual HRESULT HandleKeyConvertWildCard(KeyHandlerEditSessionDTO dto);

    // HandleKeyCancel
    virtual HRESULT HandleKeyCancel(KeyHandlerEditSessionDTO dto);

    // HandleKeyBackspace
    virtual HRESULT HandleKeyBackspace(KeyHandlerEditSessionDTO dto);

    // HandleKeyArrow
    virtual HRESULT HandleKeyArrow(KeyHandlerEditSessionDTO dto);

    // HandleKeyDoubleSingleByte
    virtual HRESULT HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto);

    // HandleKeyPunctuation
    virtual HRESULT HandleKeyPunctuation(KeyHandlerEditSessionDTO dto);

    // HandleKeySelectByNumber
    virtual HRESULT HandleKeySelectByNumber(KeyHandlerEditSessionDTO dto);

protected:
    CSampleIME* _pTextService;
};

class CKeyStateComposing : public CKeyStateCategory
{
public:
    CKeyStateComposing(_In_ CSampleIME *pTextService);

protected:
    // _HandleCompositionInput
    HRESULT HandleKeyInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyCompositionFinalizeTextStoreAndInput
    HRESULT HandleKeyFinalizeTextStoreAndInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyFinalizeTextStore
    HRESULT HandleKeyFinalizeTextStore(KeyHandlerEditSessionDTO dto);

    // HandleKeyCompositionFinalizeCandidatelistAndInput
    HRESULT HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto);

    // HandleKeyCompositionFinalizeCandidatelist
    HRESULT HandleKeyFinalizeCandidatelist(KeyHandlerEditSessionDTO dto);

    // HandleCompositionConvert
    HRESULT HandleKeyConvert(KeyHandlerEditSessionDTO dto);

    // HandleKeyCompositionConvertWildCard
    HRESULT HandleKeyConvertWildCard(KeyHandlerEditSessionDTO dto);

    // HandleCancel
    HRESULT HandleKeyCancel(KeyHandlerEditSessionDTO dto);

    // HandleCompositionBackspace
    HRESULT HandleKeyBackspace(KeyHandlerEditSessionDTO dto);

    // HandleArrowKey
    HRESULT HandleKeyArrow(KeyHandlerEditSessionDTO dto);

    // HandleKeyDoubleSingleByte
    HRESULT HandleKeyDoubleSingleByte(KeyHandlerEditSessionDTO dto);

    // HandleKeyCompositionPunctuation
    HRESULT HandleKeyPunctuation(KeyHandlerEditSessionDTO dto);
};

class CKeyStateCandidate : public CKeyStateCategory
{
public:
    CKeyStateCandidate(_In_ CSampleIME *pTextService);

protected:
    // HandleKeyFinalizeCandidatelist
    HRESULT HandleKeyFinalizeCandidatelist(KeyHandlerEditSessionDTO dto);

    // HandleKeyFinalizeCandidatelistAndInput
    HRESULT HandleKeyFinalizeCandidatelistAndInput(KeyHandlerEditSessionDTO dto);

    //_HandleCandidateConvert
    HRESULT HandleKeyConvert(KeyHandlerEditSessionDTO dto);

    //_HandleCancel
    HRESULT HandleKeyCancel(KeyHandlerEditSessionDTO dto);

    //_HandleCandidateArrowKey
    HRESULT HandleKeyArrow(KeyHandlerEditSessionDTO dto);

    //_HandleCandidateSelectByNumber
    HRESULT HandleKeySelectByNumber(KeyHandlerEditSessionDTO dto);
};

//degeneration class
class CKeyStateNull : public CKeyStateCategory
{
public:
    CKeyStateNull(_In_ CSampleIME *pTextService) : CKeyStateCategory(pTextService) {};
};
