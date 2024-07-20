// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _V8APP_SNAPSHOT_PROVIDER_H_
#define _V8APP_SNAPSHOT_PROVIDER_H_

#include <string>

#include "V8Types.h"
#include "IJSSnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Class that handles loading the snapshot data for the standard v8 data.
         * Ovderride this class to implement custom snapshots to handle the serialization custom format.
         *
         * File Format
         * |   4 Bytes    | 0 used to detect if it's a v8 created snapshot since it has a magic number at the start
         * |   4 Bytes    | Magic Number for v8App
         * | 4 or 8 bytes | length of app version
         * |     ...      | content of version string
         * | 4 or 8 bytes | length of runtime version
         * |     ...      | content of runtime version
         * | 4 or 8 bytes | length of architecture
         * |     ...      | content of architecture string
         * |     ...      | JSApp Data
         * |     ...      |     Runtime NamedIndexes the number that are serialized is number of snapshot below
         * ---------------- Snapshot Section
         * | 4 or 8 bytes | snapshot blob size
         * |     ...      | v8 snapshot blob created by the app
         * |     ...      | 
         * |     ...      | JSRuntime data
         * |     ...      |     COntext NamedIndex
         * ----------------
         * |     ...      | n snapshot
         * ----------------
         */
        class V8AppSnapshotProvider : public IJSSnapshotProvider
        {
        public:
            V8AppSnapshotProvider() = default;
            virtual ~V8AppSnapshotProvider() = default;

            virtual bool LoadSnapshotData(JSAppSharedPtr inApp, std::filesystem::path) override;

            /**
             * Gets the v8 startup data that the isolate needs
             */
            virtual const V8StartupData *GetSnapshotData(size_t inIndex = 0) override;

            virtual const intptr_t *GetExternalReferences() override;

            /**
             * Serializers for the snapshot
             */
            virtual V8StartupData SerializeInternalField(V8LObject inHolder, int inIndex) override;
            virtual V8StartupData SerializeContextInternalField(V8LContext inHolder, int inIndex) override;

            /**
             * Deserializers for the snapshot
             */
            virtual void DeserializeInternalField(V8LObject inHolder, int inIndex, V8StartupData inPayload) override;
            virtual void DeserializeContextInternalField(V8LContext inHolder, int inIndex, V8StartupData inPayload) override;

        protected:
            /**
             * Loads the data file
             */
            bool LoadDataFile(JSAppSharedPtr inApp, std::filesystem::path inSnapshotPath);

            using SnapshotMap = std::map<std::string, V8StartupData>;

            SnapshotMap m_Snapshots;
        };
    }
}

#endif //_V8APP_SNAPSHOT_PROVIDER_H_