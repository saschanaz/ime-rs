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

VOID CTableDictionaryEngine::CollectWord(const CStringRangeSmart& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, keyCode);

    while (dshSearch.FindPhrase(&pdret))
    {
        CCandidateListItem listItem;
        listItem._ItemString.Set(pdret->_FoundPhrase);
        listItem._FindKeyCode.Set(pdret->_FindKeyCode);
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

VOID CTableDictionaryEngine::CollectWordForWildcard(const CStringRangeSmart& keyCode, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, keyCode);

    while (dshSearch.FindPhraseForWildcard(&pdret))
    {
        CCandidateListItem listItem;
        listItem._ItemString.Set(pdret->_FoundPhrase);
        listItem._FindKeyCode.Set(pdret->_FindKeyCode);
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

VOID CTableDictionaryEngine::CollectWordFromConvertedStringForWildcard(const CStringRangeSmart& string, _Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    CDictionaryResult* pdret = nullptr;
    CDictionarySearch dshSearch(_locale, _pDictionaryFile, string);

    while (dshSearch.FindConvertedStringForWildcard(&pdret)) // TAIL ALL CHAR MATCH
    {
        CCandidateListItem listItem;
        listItem._ItemString.Set(pdret->_FoundPhrase);
        listItem._FindKeyCode.Set(pdret->_FindKeyCode);
        pItemList->Append(listItem);

        delete pdret;
        pdret = nullptr;
    }
}

