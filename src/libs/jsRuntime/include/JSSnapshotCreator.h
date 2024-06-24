// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __JS_SNAPSHOT_CREATOR_H__
#define __JS_SNAPSHOT_CREATOR_H__

#include <filesystem>

#include "V8Types.h"
#include "JSApp.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSSnapshotCreator
        {
        public:
            JSSnapshotCreator(JSAppSharedPtr inApp);
            ~JSSnapshotCreator();

            bool CreateSnapshotFile(std::filesystem::path inSnapshotFile);

        protected:
            JSAppSharedPtr m_App;
        };
    }
}

#endif //__JS_SNAPSHOT_CREATOR_H__