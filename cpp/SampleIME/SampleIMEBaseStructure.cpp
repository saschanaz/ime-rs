// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <codecvt>

#include "Globals.h"
#include "RustStringRange.h"

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "userenv.lib")
// Rust 1.57+ needs bcrypt
// https://github.com/rust-lang/rust/pull/84096
#pragma comment(lib, "bcrypt.lib")
// Rust 1.70+ needs ntdll
// https://github.com/rust-lang/rust/issues/115813
// https://github.com/rust-lang/rust/pull/108262
#pragma comment(lib, "ntdll.lib")

const DWORD_PTR CStringRangeUtf16::GetLength() const
{
    return _stringBufLen;
}

const WCHAR *CStringRangeUtf16::GetRaw() const
{
    return _pStringBuf.get();
}

void CStringRangeUtf16::SetClone(const WCHAR *pwch, DWORD_PTR dwLength)
{
    _stringBufLen = dwLength;
    _pStringBuf = std::shared_ptr<WCHAR>(Clone(pwch, _stringBufLen));
}

CStringRangeUtf16::CStringRangeUtf16(WCHAR wch)
{
    _stringBufLen = 1;
    _pStringBuf = std::make_shared<WCHAR>(wch);
}

CStringRangeUtf16::CStringRangeUtf16(const CRustStringRange& rsr) {
    // The conversion to UTF16 is in C++ on purpose to allow easier memory management
    char* firstChar = (char*)rsr.GetRawUtf8();
    char* afterLastChar = firstChar + rsr.GetLengthUtf8();
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> conversion;
    std::u16string strU16 = conversion.from_bytes(firstChar, afterLastChar);

    SetClone((WCHAR*)strU16.c_str(), strU16.length());
}

CStringRangeUtf16::operator CRustStringRange() const {
    return CRustStringRange(GetRaw(), GetLength());
};

WCHAR* CStringRangeUtf16::Clone(const WCHAR* pwch, DWORD_PTR dwLength)
{
    if (!dwLength) {
        return nullptr;
    }
    WCHAR* pwchString = new (std::nothrow) WCHAR[ dwLength ];
    if (!pwchString)
    {
        return nullptr;
    }
    memcpy((void*)pwchString, pwch, dwLength * sizeof(WCHAR));
    return pwchString;
}
