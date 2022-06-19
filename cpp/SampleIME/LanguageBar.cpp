// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "SampleIME.h"
#include "CompositionProcessorEngine.h"
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

    _pCompositionProcessorEngine->DisableLanguageBarButton(needDisableButtons);
}
