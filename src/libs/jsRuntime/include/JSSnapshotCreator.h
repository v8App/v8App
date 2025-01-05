// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __JS_SNAPSHOT_CREATOR_H__
#define __JS_SNAPSHOT_CREATOR_H__

#include <filesystem>

#include "V8Types.h"
#include "JSApp.h"
#include "IJSContextProvider.h"
#include "V8SnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSSnapshotCreator
        {
        public:
            JSSnapshotCreator(JSAppSharedPtr inApp);
            ~JSSnapshotCreator();

            /**
             * Template function to create the JSApp specified class setup for snapshotting
             * This created the app and the runtime but no contextes.
             * JSApp has a version but it creates a clone of the app setup for snapshotting
             */
            template<class App>
            static JSAppSharedPtr CreateSnapshotApp(std::string inName, std::filesystem::path inAppRoot,
                                                    AppProviders inAppProviders);

            bool CreateSnapshotFile(std::filesystem::path inSnapshotFile);

        protected:
            JSAppSharedPtr m_App;
        };

    }
}

#include "JSSnapshotCreator.hpp"

#endif //__JS_SNAPSHOT_CREATOR_H__