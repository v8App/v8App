// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __TEST_SNAPSHOT_CREATOR__
#define __TEST_SNAPSHOT_CREATOR__

#include "IJSSnapshotCreator.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Test version that doesn't really do anything other than testing the log around
         * whether the snapshot is created or not
         */
        class TestSnapshotCreator : public IJSSnapshotCreator
        {
        public:
            virtual bool CreateSnapshot(ISnapshotObject &inObject, std::filesystem::path inSnapshotFile) { return m_Success; }

            /**
             * Gets the external references that the snapshot will need.
             * Default v8 doesn't need any
             */
            virtual const intptr_t *GetExternalReferences() { return nullptr; }

            /**
             * Serializers for the snapshot
             */
            virtual V8StartupData SerializeInternalField(V8LObject inHolder, int inIndex) { return {nullptr, 0}; }
            virtual V8StartupData SerializeAPIWrapperField(V8LObject inHolder, CppBridge::V8CppObjectBase* inObject) { return {nullptr, 0}; }
            virtual V8StartupData SerializeContextInternalField(V8LContext inHolder, int inIndex) { return {nullptr, 0}; }

            void SetSuccess(bool inSuccess) { m_Success = inSuccess; }

        protected:
            bool m_Success{false};
        };
    }
}

#endif //__TEST_SNAPSHOT_CREATOR__