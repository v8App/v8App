// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "JSSnapshotCreator.h"

namespace v8App
{
    namespace JSRuntime
    {
        template <class App>
        JSAppSharedPtr JSSnapshotCreator::CreateSnapshotApp(std::string inName, std::filesystem::path inAppRoot,
                                                            AppProviders inAppProviders)
        {
            JSAppSharedPtr app = std::make_shared<App>(inName, inAppProviders);
            if (app->Initialize(inAppRoot, true) == false)
            {
                return nullptr;
            }
            /** 
            if (app->AppInit() == false)
            {
                return nullptr;
            }
            */
            return app;
        }

    }
}
