// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _V8APP_SNAPSHOT_PROVIDER_H_
#define _V8APP_SNAPSHOT_PROVIDER_H_

#include <string>
#include <map>

#include "Assets/AppAssetRoots.h"

#include "V8Types.h"
#include "CppBridge/CallbackRegistry.h"
#include "V8SnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Class that handles loading the snapshot data for the standard v8 data.
         * Ovderride this class to implement custom snapshots to handle the serialization custom format.
         */
        class V8AppSnapshotProvider : public V8SnapshotProvider
        {
        public:
            V8AppSnapshotProvider(std::filesystem::path inSnapshotPath = std::filesystem::path()) : V8SnapshotProvider(inSnapshotPath) {}
            virtual ~V8AppSnapshotProvider() = default;

            virtual bool LoadSnapshotData(JSAppSharedPtr inApp, std::filesystem::path = std::filesystem::path()) override;

            virtual const intptr_t* GetExternalReferences() override { return CppBridge::CallbackRegistry::GetReferences().data(); }

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
             * Has the data been loaded
             */
            bool m_Loaded = false;
            /**
             * Snapshot data
             */
            V8StartupData m_StartupData{nullptr, 0};
            /**
             * Path of teh file the data was loaded from
             */
            std::filesystem::path m_SnapshotPath;
        };
    }
}

#endif //_V8APP_SNAPSHOT_PROVIDER_H_