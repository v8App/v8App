// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __TEST_SNAPSHOT_PROVIDER__
#define __TEST_SNAPSHOT_PROVIDER__

#include "IJSSnapshotProvider.h"
#include "CppBridge/CallbackRegistry.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestSnapshotProvider : public IJSSnapshotProvider
        {
        public:
            TestSnapshotProvider();
            virtual bool LoadSnapshotData(std::filesystem::path inSnapshotPath = std::filesystem::path()) override;
            virtual const V8StartupData *GetSnapshotData(size_t inIndex = 0) override;

            /**
             * The test provider uses the stndard V8 Sanpshot so there is only one runtime
             */
            virtual size_t GetIndexForRuntimeName(std::string inRuntimeName) override { return 0; };
            /**
             * The test provider uses the stadnard V8 Sanpshot just has the default context
             */
            virtual size_t GetIndexForContextName(std::string inName, std::string inRuntimeName) override { return 0; };
            virtual size_t GetIndexForContextName(std::string inName, size_t inRuntimeIndex) override { return 0; };

            virtual bool IsRuntimeIndexValid(size_t inIndex) override { return inIndex == 0; };

            virtual bool IsContextIndexValid(size_t inIndex, std::string inRuntimeName) override { return inIndex == 0; };
            virtual bool IsContextIndexValid(size_t inIndex, size_t inRuntimeIndex) override { return inIndex == 0; }

            virtual const intptr_t *GetExternalReferences() override
            {
                return CppBridge::CallbackRegistry::GetReferences().data();
            }

            virtual const JSAppSnapDataSharedPtr GetJSAppSnapData() override { return nullptr; }

            virtual void DeserializeInternalField(V8LObject inHolder, int inIndex, V8StartupData inPayload) override {};
            virtual void DeserializeAPIWrapperField(V8LObject inHolder, V8StartupData inPayload) override {};
            virtual void DeserializeContextInternalField(V8LContext inHolder, int inIndex, V8StartupData inPayload, JSContext *inJSContext) override {};

            /**
             * Test provider can't restore a snapshot
             */
            virtual JSAppSharedPtr RestoreApp(std::filesystem::path inAppRoot, AppProviders inProviders) override { return nullptr; }

            void SetLoaded(bool inLoaded) { m_Loaded = inLoaded; }
            void SetReturnEmpty(bool inLoaded) { m_ReturnEmpty = inLoaded; }

        protected:
            bool m_ReturnEmpty = false;
            V8StartupData m_TestStartup{nullptr, 0};
        };
    }
}

#endif //__TEST_SNAPSHOT_PROVIDER__