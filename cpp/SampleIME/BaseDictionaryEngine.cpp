// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "BaseDictionaryEngine.h"
#include "Globals.h"

//+---------------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------------

CBaseDictionaryEngine::CBaseDictionaryEngine(LCID locale, _In_ CFile *pDictionaryFile)
{
    _locale = locale;
    _pDictionaryFile = pDictionaryFile;
}

//+---------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------

CBaseDictionaryEngine::~CBaseDictionaryEngine()
{
}

//+---------------------------------------------------------------------------
// SortListItemByFindKeyCode
//----------------------------------------------------------------------------

VOID CBaseDictionaryEngine::SortListItemByFindKeyCode(_Inout_ CSampleImeArray<CCandidateListItem> *pItemList)
{
    MergeSortByFindKeyCode(pItemList, 0, pItemList->Count() - 1);
}

//+---------------------------------------------------------------------------
// MergeSortByFindKeyCode
//
//    Mergesort the array of element in CCandidateListItem::_FindKeyCode
//
//----------------------------------------------------------------------------

VOID CBaseDictionaryEngine::MergeSortByFindKeyCode(_Inout_ CSampleImeArray<CCandidateListItem> *pItemList, int leftRange, int rightRange)
{
    int candidateCount = CalculateCandidateCount(leftRange, rightRange);

    if (candidateCount > 2)
    {
        int mid = leftRange + (candidateCount / 2);

        MergeSortByFindKeyCode(pItemList, leftRange, mid);
        MergeSortByFindKeyCode(pItemList, mid, rightRange);

        CSampleImeArray<CCandidateListItem> ListItemTemp;

        int leftRangeTemp = 0;
        int midTemp = 0;
        for (leftRangeTemp = leftRange, midTemp = mid; leftRangeTemp != mid || midTemp != rightRange;)
        {
            CStringRangeSmart srgLeftTemp;
            CStringRangeSmart srgMidTemp;

            srgLeftTemp = pItemList->GetAt(leftRangeTemp)->_FindKeyCode;
            srgMidTemp = pItemList->GetAt(midTemp)->_FindKeyCode;

            CCandidateListItem* pLI = nullptr;
            pLI = ListItemTemp.Append();
            if (pLI)
            {
                if (leftRangeTemp == mid)
                {
                    *pLI = *pItemList->GetAt(midTemp++);
                }
                else if (midTemp == rightRange || CStringRangeSmart::Compare(_locale, srgLeftTemp, srgMidTemp) != CSTR_GREATER_THAN)
                {
                    *pLI = *pItemList->GetAt(leftRangeTemp++);
                }
                else
                {
                    *pLI = *pItemList->GetAt(midTemp++);
                }
            }
        }

        leftRangeTemp = leftRange;
        for (const auto& item : ListItemTemp)
        {
            *pItemList->GetAt(leftRangeTemp++) = item;
        }
    }
    else if (candidateCount == 2)
    {
        CStringRangeSmart srgLeft;
        CStringRangeSmart srgLeftNext;

        srgLeft = pItemList->GetAt(leftRange )->_FindKeyCode;
        srgLeftNext = pItemList->GetAt(leftRange+1)->_FindKeyCode;

        if (CStringRangeSmart::Compare(_locale, srgLeft, srgLeftNext) == CSTR_GREATER_THAN)
        {
            CCandidateListItem ListItem;
            ListItem = *pItemList->GetAt(leftRange);
            *pItemList->GetAt(leftRange ) = *pItemList->GetAt(leftRange+1);
            *pItemList->GetAt(leftRange+1) = ListItem;
        }
    }
}

int CBaseDictionaryEngine::CalculateCandidateCount(int leftRange,  int rightRange)
{
    assert(leftRange >= 0);
    assert(rightRange >= 0);

    return (rightRange - leftRange + 1);
}
