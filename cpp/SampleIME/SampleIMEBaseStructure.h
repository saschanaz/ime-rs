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
#include "cbindgen/composition_processor.h"
#include "cbindgen/numberkey_windows.h"

using std::cout;
using std::endl;

//---------------------------------------------------------------------
// structure
//---------------------------------------------------------------------
struct _KEYSTROKE_STATE
{
    KeystrokeCategory Category;
    KeystrokeFunction Function;
};

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
    CCandidateRange() = delete;
    ~CCandidateRange() = delete;

    static int16_t GetIndex(UINT vKey) {
        return index_from_number_key(vKey);
    };

    static const int Count = 10;
    static inline uint16_t GetAt(int index)
    {
        return number_key_label_at(index);
    }
};

class CStringRangeUtf16
{
public:
    CStringRangeUtf16(WCHAR wch);
    explicit CStringRangeUtf16(const CRustStringRange& rsr);
    ~CStringRangeUtf16() {};

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
