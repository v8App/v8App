// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iomanip>
#include <fstream>

#include "TestSnapshotProvider.h"
#include "test_main.h"

namespace v8App
{
    namespace JSRuntime
    {
        bool TestSnapshotProvider::LoadSnapshotData(std::filesystem::path inSnaopshotFile, JSAppSharedPtr inApp)
        {
            //the main test function loads it for us
            return true;
        }

        const v8::StartupData *TestSnapshotProvider::GetSnapshotData()
        {
            return &s_V8StartupData;
        }
    }
}