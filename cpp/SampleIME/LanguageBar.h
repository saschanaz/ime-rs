// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

class RustLangBarItemButton {
public:
    static ITfLangBarItemButton* New();
    static HRESULT Init(ITfLangBarItemButton* button, ITfThreadMgr* threadMgr, TfClientId clientId, const GUID* guidCompartment);
    static void Cleanup(ITfLangBarItemButton* button);
    static HRESULT SetStatus(ITfLangBarItemButton* button, DWORD status, BOOL fSet);
};
