// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __TEST_SNAPSHOT_PROVIDER__
#define __TEST_SNAPSHOT_PROVIDER__

#include "IJSSnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestSnapshotProvider : public IJSSnapshotProvider
        {
        public:
            TestSnapshotProvider() { m_Loaded = true; }
            virtual bool LoadSnapshotData(JSAppSharedPtr inApp, std::filesystem::path inSnapshotPath = std::filesystem::path()) override;
            virtual const V8StartupData *GetSnapshotData(size_t inIndex = 0) override;
            virtual const intptr_t *GetExternalReferences() override { return nullptr; }
            virtual void DeserializeInternalField(V8LObject inHolder, int inIndex, V8StartupData inPayload) override {};
            virtual void DeserializeContextInternalField(V8LContext inHolder, int inIndex, V8StartupData inPayload) override {};

            void SetLoaded(bool inLoaded) { m_Loaded = inLoaded; }
            void SetReturnEmpty(bool inLoaded) { m_ReturnEmpty = inLoaded; }

        protected:
            bool m_ReturnEmpty = false;
            V8StartupData m_TestStartup{nullptr, 0};
        };
    }
}

#endif //__TEST_SNAPSHOT_PROVIDER__