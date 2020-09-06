// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include "File.h"
#include "DictionaryParser.h"
#include "SampleIMEBaseStructure.h"
#include "RustStringRange.h"

class CDictionaryResult;

//////////////////////////////////////////////////////////////////////
//
// CDictionarySearch declaration.
//
//////////////////////////////////////////////////////////////////////

class CDictionarySearch : CDictionaryParser
{
public:
    CDictionarySearch(LCID locale, _In_ CFile *pFile, const CStringRangeSmart& searchKeyCode);
    virtual ~CDictionarySearch();

    BOOL FindPhrase(_Out_ CDictionaryResult **ppdret);
    BOOL FindPhraseForWildcard(_Out_ CDictionaryResult **ppdret);

    BOOL FindConvertedStringForWildcard(CDictionaryResult **ppdret);

    CRustStringRange _searchKeyCode;

    DWORD_PTR _charIndex;      // in character. Always point start of line in dictionary file.

private:
    BOOL FindWorker(BOOL isTextSearch, _Out_ CDictionaryResult **ppdret, BOOL isWildcardSearch);

    DWORD_PTR GetBufferInCharLength()
    {
        return (_pFile->GetFileSize() / sizeof(char)) - _charIndex;     // in char count as a returned length.
    }

    const char* GetBufferInChar()
    {
        return _pFile->GetReadBufferPointer() + _charIndex;
    }

    CFile* _pFile;
};

//////////////////////////////////////////////////////////////////////
//
// CDictionaryResult declaration.
//
//////////////////////////////////////////////////////////////////////

class CDictionaryResult
{
public:
    CDictionaryResult() { }
    virtual ~CDictionaryResult() { }

    CDictionaryResult& operator=(CDictionaryResult& dret)
    {
        _FindKeyCode = dret._FindKeyCode;
        _FoundPhrase = dret._FoundPhrase;
        return *this;
    }

    CStringRangeSmart _SearchKeyCode;
    CStringRangeSmart _FindKeyCode;
    CStringRangeSmart _FoundPhrase;
};
