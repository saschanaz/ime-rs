// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "stdafx.h"
#include <vector>
#include "assert.h"
#include <iostream>
#include "RustStringRange.h"

using std::cout;
using std::endl;

//---------------------------------------------------------------------
// defined keyword
//---------------------------------------------------------------------
template<class VALUE>
struct _DEFINED_KEYWORD
{
    LPCWSTR _pwszKeyword;
    VALUE _value;
};

//---------------------------------------------------------------------
// enum
//---------------------------------------------------------------------
enum KEYSTROKE_CATEGORY
{
    CATEGORY_NONE = 0,
    CATEGORY_COMPOSING,
    CATEGORY_CANDIDATE,
    CATEGORY_PHRASE,
    CATEGORY_PHRASEFROMKEYSTROKE,
    CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION
};

enum KEYSTROKE_FUNCTION
{
    FUNCTION_NONE = 0,
    FUNCTION_INPUT,

    FUNCTION_CANCEL,
    FUNCTION_FINALIZE_TEXTSTORE,
    FUNCTION_FINALIZE_TEXTSTORE_AND_INPUT,
    FUNCTION_FINALIZE_CANDIDATELIST,
    FUNCTION_FINALIZE_CANDIDATELIST_AND_INPUT,
    FUNCTION_CONVERT,
    FUNCTION_CONVERT_WILDCARD,
    FUNCTION_SELECT_BY_NUMBER,
    FUNCTION_BACKSPACE,
    FUNCTION_MOVE_LEFT,
    FUNCTION_MOVE_RIGHT,
    FUNCTION_MOVE_UP,
    FUNCTION_MOVE_DOWN,
    FUNCTION_MOVE_PAGE_UP,
    FUNCTION_MOVE_PAGE_DOWN,
    FUNCTION_MOVE_PAGE_TOP,
    FUNCTION_MOVE_PAGE_BOTTOM,

    // Function Double/Single byte
    FUNCTION_DOUBLE_SINGLE_BYTE,

    // Function Punctuation
    FUNCTION_PUNCTUATION
};

//---------------------------------------------------------------------
// candidate list
//---------------------------------------------------------------------
enum CANDIDATE_MODE
{
    CANDIDATE_NONE = 0,
    CANDIDATE_ORIGINAL,
    CANDIDATE_PHRASE,
    CANDIDATE_INCREMENTAL,
    CANDIDATE_WITH_NEXT_COMPOSITION
};

//---------------------------------------------------------------------
// structure
//---------------------------------------------------------------------
struct _KEYSTROKE_STATE
{
    KEYSTROKE_CATEGORY Category;
    KEYSTROKE_FUNCTION Function;
};

struct _PUNCTUATION
{
    WCHAR _Code;
    WCHAR _Punctuation;
};

BOOL CLSIDToString(REFGUID refGUID, _Out_writes_ (39) WCHAR *pCLSIDString);

HRESULT SkipWhiteSpace(LCID locale, _In_ LPCWSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex);
HRESULT FindChar(char wch, _In_ LPCSTR pwszBuffer, DWORD_PTR dwBufLen, _Out_ DWORD_PTR *pdwIndex);

BOOL IsSpace(LCID locale, WCHAR wch);

template<class T>
class CSampleImeArray
{
    typedef typename std::vector<T> CSampleImeInnerArray;
    typedef typename std::vector<T>::iterator CSampleImeInnerIter;

public:
    CSampleImeArray(): _innerVect()
    {
    }

    explicit CSampleImeArray(size_t count): _innerVect(count)
    {
    }

    virtual ~CSampleImeArray()
    {
    }

    inline T* GetAt(size_t index)
    {
        assert(index >= 0);
        assert(index < _innerVect.size());

        T& curT = _innerVect.at(index);

        return &(curT);
    }

    inline const T* GetAt(size_t index) const
    {
        assert(index >= 0);
        assert(index < _innerVect.size());

        T& curT = _innerVect.at(index);

        return &(curT);
    }

    void RemoveAt(size_t index)
    {
        assert(index >= 0);
        assert(index < _innerVect.size());

        CSampleImeInnerIter iter = _innerVect.begin();
        _innerVect.erase(iter + index);
    }

    UINT Count() const
    {
        return static_cast<UINT>(_innerVect.size());
    }

    void Append(T item)
    {
        _innerVect.push_back(item);
    }

    void reserve(size_t Count)
    {
        _innerVect.reserve(Count);
    }

    void Clear()
    {
        _innerVect.clear();
    }

    auto begin() {
        return _innerVect.begin();
    }

    auto end() {
        return _innerVect.end();
    }

private:
    CSampleImeInnerArray _innerVect;
};

class CCandidateRange
{
public:
    CCandidateRange(void);
    ~CCandidateRange(void);

    BOOL IsRange(UINT vKey);
    int GetIndex(UINT vKey);

    inline int Count() const
    {
        return _CandidateListIndexRange.Count();
    }
    inline DWORD *GetAt(int index)
    {
        return _CandidateListIndexRange.GetAt(index);
    }
    inline void Append(DWORD dw)
    {
        _CandidateListIndexRange.Append(dw);
    }

private:
    CSampleImeArray<DWORD> _CandidateListIndexRange;
};

class CStringRangeSmart
{
public:
    CStringRangeSmart(WCHAR wch);
    explicit CStringRangeSmart(const CRustStringRange& rsr);
    ~CStringRangeSmart() {};

    const DWORD_PTR GetLength() const;
    const WCHAR *GetRaw() const;

    explicit operator CRustStringRange() const;

protected:
    void SetClone(const WCHAR *pwch, DWORD_PTR dwLength);
    static WCHAR* Clone(const WCHAR* pwch, DWORD_PTR dwLength);

    std::shared_ptr<const WCHAR> _pStringBuf;    // Buffer which is not add zero terminate.
    DWORD_PTR _stringBufLen = 0;         // Length is in character count.
};

//---------------------------------------------------------------------
// CCandidateListItem
//	_ItemString - candidate string
//	_FindKeyCode - tailing string
//---------------------------------------------------------------------
struct CCandidateListItem
{
    CCandidateListItem(CRustStringRange itemString, CRustStringRange findKeyCode): _ItemString(itemString), _FindKeyCode(findKeyCode) {}

    CRustStringRange _ItemString;
    CRustStringRange _FindKeyCode;

	CCandidateListItem& CCandidateListItem::operator =( const CCandidateListItem& rhs)
	{
		_ItemString = rhs._ItemString;
		_FindKeyCode = rhs._FindKeyCode;
		return *this;
	}
};

class CPunctuationPair
{
public:
    CPunctuationPair();
    CPunctuationPair(WCHAR code, WCHAR punctuation, WCHAR pair);

    struct _PUNCTUATION _punctuation;
    WCHAR _pairPunctuation;
    BOOL _isPairToggle;
};

class CPunctuationNestPair
{
public:
    CPunctuationNestPair();
    CPunctuationNestPair(WCHAR wchCode_begin, WCHAR wch_begin, WCHAR wchPair_begin,
        WCHAR wchCode_end,   WCHAR wch_end,   WCHAR wchPair_end);

    struct _PUNCTUATION _punctuation_begin;
    WCHAR _pairPunctuation_begin;

    struct _PUNCTUATION _punctuation_end;
    WCHAR _pairPunctuation_end;

    int _nestCount;
};
