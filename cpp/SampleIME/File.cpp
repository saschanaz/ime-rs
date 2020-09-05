// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "File.h"
#include "SampleIMEBaseStructure.h"

//---------------------------------------------------------------------
//
// ctor
//
//---------------------------------------------------------------------

CFile::CFile(UINT codePage)
{
    _codePage = codePage;
    _fileHandle = nullptr;
    _pReadBuffer = nullptr;
    _fileSize = 0;
    _filePosPointer = 0;
    _pFileName = nullptr;
}

//---------------------------------------------------------------------
//
// dtor
//
//---------------------------------------------------------------------

CFile::~CFile()
{
    if (_pReadBuffer)
    {
        delete [] _pReadBuffer;
        _pReadBuffer = nullptr;
    }
    if (_fileHandle)
    {
        CloseHandle(_fileHandle);
        _fileHandle = nullptr;
    }
    if (_pFileName)
    {
        delete [] _pFileName;
        _pFileName = nullptr;
    }
}

//---------------------------------------------------------------------
//
// CreateFile
//
//---------------------------------------------------------------------

BOOL CFile::CreateFile(_In_ PCWSTR pFileName, DWORD desiredAccess,
    DWORD creationDisposition,
    DWORD sharedMode, _Inout_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD flagsAndAttributes, _Inout_opt_ HANDLE templateFileHandle)
{
    size_t fullPathLen = wcslen(pFileName);
    if (!_pFileName)
    {
        _pFileName = new (std::nothrow) WCHAR[ fullPathLen + 1 ];
    }
    if (!_pFileName)
    {
        return FALSE;
    }

    StringCchCopyN(_pFileName, fullPathLen + 1, pFileName, fullPathLen);

    _fileHandle = ::CreateFile(pFileName, desiredAccess, sharedMode,
        lpSecurityAttributes, creationDisposition, flagsAndAttributes, templateFileHandle);

    if (_fileHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    _fileSize = ::GetFileSize(_fileHandle, NULL);

    return TRUE;
}

//---------------------------------------------------------------------
//
// SetupReadBuffer
//
//---------------------------------------------------------------------

BOOL CFile::SetupReadBuffer()
{
    _pReadBuffer = (const char *) new (std::nothrow) BYTE[ _fileSize ];
    if (!_pReadBuffer)
    {
        return FALSE;
    }

    DWORD dwNumberOfByteRead = 0;
    if (!ReadFile(_fileHandle, (LPVOID)_pReadBuffer, (DWORD)_fileSize, &dwNumberOfByteRead, NULL))
    {
        delete [] _pReadBuffer;
        _pReadBuffer = nullptr;
        return FALSE;
    }

    if (!_fileSize) {
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// IsEndOfFile
//
//----------------------------------------------------------------------------

BOOL CFile::IsEndOfFile()
{
    return _fileSize == _filePosPointer ? TRUE : FALSE;
}

//+---------------------------------------------------------------------------
//
// NextLine
//
//----------------------------------------------------------------------------

VOID CFile::NextLine()
{
    DWORD_PTR totalBufLen = GetBufferInCharLength();
    if (totalBufLen == 0)
    {
        goto SetEOF;
    }
    const char *pch = GetBufferInChar();

    DWORD_PTR indexTrace = 0;       // in char

    if (FindChar(u8'\r', pch, totalBufLen, &indexTrace) != S_OK)
    {
        goto SetEOF;
    }
    if (indexTrace >= DWORD_MAX -1)
    {
        goto SetEOF;
    }

    indexTrace++;  // skip CR
    totalBufLen -= indexTrace;
    if (totalBufLen == 0)
    {
        goto SetEOF;
    }

    if (pch[indexTrace] != u8'\n')
    {
        _filePosPointer += (indexTrace * sizeof(char));
        return;
    }

    indexTrace++;
    totalBufLen--;
    if (totalBufLen == 0)
    {
        goto SetEOF;
    }

    _filePosPointer += (indexTrace * sizeof(char));

    return;

SetEOF:
    _filePosPointer = _fileSize;
    return;
}
