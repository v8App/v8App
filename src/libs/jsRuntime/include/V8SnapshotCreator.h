// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _V8SNAPSHOT_CREATOR_H__
#define _V8SNAPSHOT_CREATOR_H__

#include "IJSSnapshotCreator.h"

namespace v8App
{
    namespace JSRuntime
    {
        /*
         * Recreates the V8 Snapshot style file
         */
        class V8SnapshotCreator : public IJSSnapshotCreator
        {
        public:
            V8SnapshotCreator() = default;
            virtual ~V8SnapshotCreator() = default;

            virtual bool CreateSnapshot(ISnapshotObject &inObject, std::filesystem::path inSnapshotFile) override;

            /**
             * Gets the external references that the snapshot will need.
             * Default v8 doesn't need any
             */
            virtual const intptr_t *GetExternalReferences() override { return nullptr; };

            /**
             * Serializers for the snapshot
             */
            virtual V8StartupData SerializeInternalField(V8LObject inHolder, int inIndex) override { return {nullptr, 0}; }
            virtual V8StartupData SerializeContextInternalField(V8LContext inHolder, int inIndex) override { return {nullptr, 0}; }
        };

    }
}
#endif //_V8SNAPSHOT_CREATOR_H__