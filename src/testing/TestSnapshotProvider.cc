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
        bool TestSnapshotProvider::LoadSnapshotData(JSAppSharedPtr inApp, std::filesystem::path inSnaopshotPath)
        {
            // the main test function loads it for us but to test code that calls it base the return in m_Loaded
            return m_Loaded;
        }

        const V8StartupData *TestSnapshotProvider::GetSnapshotData(size_t inIndex)
        {
            // if were loaded then return the data other wise return
            if (m_ReturnEmpty)
            {
                return &m_TestStartup;
            }
            return &s_V8StartupData;
        }
    }
}