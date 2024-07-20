// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "JSApp.h"

namespace v8App
{
    namespace JSRuntime
    {
        template <class App>
        JSAppSharedPtr JSApp::CloneAppForSnapshotting(IJSSnapshotProviderSharedPtr inSnapProvider)
        {
            if (m_IsSnapshotter)
            {
                return shared_from_this();
            }
            if (inSnapProvider == nullptr)
            {
                inSnapProvider = m_AppProviders.m_SnapshotProvider;
            }
            /** 
            JSAppSharedPtr snapApp = std::make_shared<App>(m_Name + "-snapshotter", inSnapProvider);
            if (snapApp->Initialize(m_AppAssets->GetAppRoot(), true, m_AppProviders) == false)
            {
                return nullptr;
            }
            if (snapApp->CloneAppForSnapshot(shared_from_this()) == false)
            {
                return nullptr;
            }
            
            return snapApp;
            */
           return nullptr;
        }

    }
}