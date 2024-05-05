// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __TEST_SNAPSHOT_PROVIDER__
#define __TEST_SNAPSHOT_PROVIDER__

#include "JSApp.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestSnapshotProvider : public JSSnapshotProvider
        {
        public:
            virtual bool LoadSnapshotData(std::filesystem::path, JSAppSharedPtr inApp) override;
            virtual const v8::StartupData *GetSnapshotData() override;
        };
    }
}

#endif //__TEST_SNAPSHOT_PROVIDER__