// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _V8APP_SNAPSHOT_CREATOR_H__
#define _V8APP_SNAPSHOT_CREATOR_H__

#include "IJSSnapshotCreator.h"

namespace v8App
{
    namespace JSRuntime
    {
        /*
         * Creates the V8App snapshot file which can contain multiple V8 snapshots
         * File Format
         * uint32 0 - Magic number for V8 to determine that it's not a actual V8 Snapshot
         * Runtime Version
         * uint32 Major Version
         * uint32 Minor Version
         * uint32 Patch Version
         * uint32 Build Version
         * JSApp Data
         */
        class V8AppSnapshotCreator : public IJSSnapshotCreator, std::enable_shared_from_this<V8AppSnapshotCreator>
        {
        public:            
            V8AppSnapshotCreator() = default;
            virtual ~V8AppSnapshotCreator() = default;

            virtual bool CreateSnapshot(ISnapshotObject &inObject, std::filesystem::path inSnapshotFile) override;

            /**
             * Gets the external references that the snapshot will need.
             * Default v8 doesn't need any
             */
            virtual const intptr_t *GetExternalReferences() override;

            /**
             * Serializers for the snapshot
             */
            virtual V8StartupData SerializeInternalField(V8LObject inHolder, int inIndex) override;
            virtual V8StartupData SerializeContextInternalField(V8LContext inHolder, int inIndex) override;
        };

    }
}
#endif //_V8APP_SNAPSHOT_CREATOR_H__