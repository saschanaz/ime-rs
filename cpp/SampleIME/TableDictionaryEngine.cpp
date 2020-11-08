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

VOID CTableDictionaryEngine::CollectWord(const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, keyCode);

    while (dshSearch.FindPhrase(&pdret))
    {
        CCandidateListItem listItem(pdret->_FoundPhrase, pdret->_FindKeyCode);
        pItemList->Append(listItem);

        delete pdret;
        pdret = nullptr;
    }
}

//+---------------------------------------------------------------------------
//
// CollectWordForWildcard
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::CollectWordForWildcard(const CRustStringRange& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, keyCode);

    while (dshSearch.FindPhraseForWildcard(&pdret))
    {
        CCandidateListItem listItem(pdret->_FoundPhrase, pdret->_FindKeyCode);
        pItemList->Append(listItem);

        delete pdret;
        pdret = nullptr;
    }
}

//+---------------------------------------------------------------------------
//
// CollectWordFromConvertedStringForWildcard
//
//----------------------------------------------------------------------------

VOID CTableDictionaryEngine::CollectWordFromConvertedStringForWildcard(const CRustStringRange& string, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, string);

    while (dshSearch.FindConvertedStringForWildcard(&pdret)) // TAIL ALL CHAR MATCH
    {
        CCandidateListItem listItem(pdret->_FoundPhrase, pdret->_FindKeyCode);
        pItemList->Append(listItem);

        delete pdret;
        pdret = nullptr;
    }
}

