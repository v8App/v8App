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
        class TestSnapshotProvider : public V8SnapshotProvider
        {
        public:
            TestSnapshotProvider(std::filesystem::path inSnapshotPath = std::filesystem::path()) : V8SnapshotProvider(inSnapshotPath) { m_Loaded = true; }
            virtual bool LoadSnapshotData(JSAppSharedPtr inApp, std::filesystem::path inSnapshotPath = std::filesystem::path()) override;
            virtual const V8StartupData *GetSnapshotData() override;
            void SetLoaded(bool inLoaded) { m_Loaded = inLoaded; }
            void SetReturnEmpty(bool inLoaded) { m_ReturnEmpty = inLoaded; }

        protected:
            bool m_ReturnEmpty = false;
            V8StartupData m_TestStartup{nullptr, 0};
        };
    }
}

#endif //__TEST_SNAPSHOT_PROVIDER__