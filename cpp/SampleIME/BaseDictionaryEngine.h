// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "File.h"
#include "SampleIMEBaseStructure.h"

class CBaseDictionaryEngine
{
public:
    CBaseDictionaryEngine(LCID locale, _In_ CFile *pDictionaryFile);
    virtual ~CBaseDictionaryEngine();

    VOID SortListItemByFindKeyCode(_Inout_ CSampleImeArray<CCandidateListItem> *pItemList);

protected:
    CFile* _pDictionaryFile;
    LCID _locale;

private:
    VOID MergeSortByFindKeyCode(_Inout_ CSampleImeArray<CCandidateListItem> *pItemList, int leftRange, int rightRange);
    int CalculateCandidateCount(int leftRange,  int rightRange);
};
