// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "TableDictionaryEngine.h"
#include "DictionarySearch.h"

//+---------------------------------------------------------------------------
//
// CollectWord
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::CollectWord(_In_ CStringRange *pKeyCode, _Inout_ CSampleImeArray<CStringRange> *pWordStrings)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, pKeyCode);

    while (dshSearch.FindPhrase(&pdret))
    {
        for (const auto& item : pdret->_FindPhraseList)
        {
            CStringRange* pPhrase = nullptr;
            pPhrase = pWordStrings->Append();
            if (pPhrase)
            {
                *pPhrase = item;
            }
        }

        delete pdret;
        pdret = nullptr;
    }
}

VOID CTableDictionaryEngine::CollectWord(_In_ CStringRange *pKeyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, pKeyCode);

    while (dshSearch.FindPhrase(&pdret))
    {
        for (auto& item : pdret->_FindPhraseList)
        {
            CCandidateListItem* pLI = nullptr;
            pLI = pItemList->Append();
            if (pLI)
            {
                pLI->_ItemString.Set(item);
                pLI->_FindKeyCode.Set(pdret->_FindKeyCode);
            }
        }

        delete pdret;
        pdret = nullptr;
    }
}

//+---------------------------------------------------------------------------
//
// CollectWordForWildcard
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::CollectWordForWildcard(_In_ CStringRange *pKeyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, pKeyCode);

    while (dshSearch.FindPhraseForWildcard(&pdret))
    {
        for (auto& item : pdret->_FindPhraseList)
        {
            CCandidateListItem* pLI = nullptr;
            pLI = pItemList->Append();
            if (pLI)
            {
                pLI->_ItemString.Set(item);
                pLI->_FindKeyCode.Set(pdret->_FindKeyCode);
            }
        }

        delete pdret;
        pdret = nullptr;
    }
}

//+---------------------------------------------------------------------------
//
// CollectWordFromConvertedStringForWildcard
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::CollectWordFromConvertedStringForWildcard(_In_ CStringRange *pString, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, pString);

    while (dshSearch.FindConvertedStringForWildcard(&pdret)) // TAIL ALL CHAR MATCH
    {
        for (auto& item : pdret->_FindPhraseList)
        {
            CCandidateListItem* pLI = nullptr;
            pLI = pItemList->Append();
            if (pLI)
            {
                pLI->_ItemString.Set(item);
                pLI->_FindKeyCode.Set(pdret->_FindKeyCode);
            }
        }

        delete pdret;
        pdret = nullptr;
    }
}

