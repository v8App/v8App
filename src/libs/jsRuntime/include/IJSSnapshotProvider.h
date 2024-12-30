// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _IJSSNAPSHOT_PROVIDER_H__
#define _IJSSNAPSHOT_PROVIDER_H__

#include <filesystem>

#include "V8Types.h"
#include "CppBridge/V8CppObjBase.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSAppSnapData;
        /*
         * Used to isolate the snapshot loading etc during testing and potentially different JS engines
         * A provider can support multiple snapshots for runtimes.
         * When snapshotting the runtime name is stored with an index to know which snapshot goes witht he name
         * so when snapshotting use names to be able to create specific runtimes based on it and when ot snapshotting
         * you can give the runtimes any name.
         * Context are also saved by their name with an index that starts at 0 for the default context that
         * has no specific setup and would be the bare context the engine provides.
         * All other Contexts should start at 1+
         */
        class IJSSnapshotProvider
        {
        public:
            IJSSnapshotProvider() = default;
            virtual ~IJSSnapshotProvider() = default;

            /**
             * Load the snapshot data file.
             * Subclasses can overrride to pull additional data out that they may have added to the file
             */
            virtual bool LoadSnapshotData(std::filesystem::path inSnapshotPath = std::filesystem::path()) = 0;
            /**
             * Gets the v8 startup data that the isolate needs
             */
            virtual const V8StartupData *GetSnapshotData(size_t inIndex = 0) = 0;
            /**
             * Gets the index for the runtime with a given name.
             */
            virtual size_t GetIndexForRuntimeName(std::string inRuntimeName) = 0;
            /**
             * Gets the context index from the context name using the runtime name
             */
            virtual size_t GetIndexForContextName(std::string inName, std::string inRuntimeName) = 0;
            /**
             * Gets the context index from the context name using the runtime index
             */
            virtual size_t GetIndexForContextName(std::string inName, size_t inRuntimeIndex) = 0;
            /**
             * Checks if the passed index is valid
             */
            virtual bool IsRuntimeIndexValid(size_t inIndex) = 0;
            /**
             * Checks if the passed index is valid
             */
            virtual bool IsContextIndexValid(size_t inIndex, std::string inRuntimeName) = 0;
            virtual bool IsContextIndexValid(size_t inIndex, size_t inRuntimeIndex) = 0;
            /**
             * Since engine providers may have a different index to the context vs what
             * is in the named index have it give us the real index.
             */
            virtual size_t RealContextIndex(size_t inNamedIndex) = 0;

            /**
             * Gets the external references that the snapshot will need.
             * Default v8 doesn't need any
             */
            virtual const intptr_t *GetExternalReferences() = 0;

            /**
             * Gets the JSApp snapshot data that was stored in in the snapshot
             */
            virtual const JSAppSnapDataSharedPtr GetJSAppSnapData() = 0;

            /**
             * Deserializers for the snapshot
             */
            virtual void DeserializeInternalField(V8LObject inHolder, int inIndex, V8StartupData inPayload) = 0;
            virtual void DeserializeAPIWrapperField(V8LObject inHolder, V8StartupData inPayload) = 0;
            virtual void DeserializeContextInternalField(V8LContext inHolder, int inIndex, V8StartupData inPayload, JSContext *inJSContext) = 0;

            /**
             * Gets the file path that the data was loaded from
             */
            std::filesystem::path GetSnapshotPath() { return m_SnapshotPath; }
            void SetSnapshotPath(std::filesystem::path inSnapPath) { m_SnapshotPath = inSnapPath; }
            /**
             * Has the snapshot data been loaded from the file yet
             */
            bool SnapshotLoaded() { return m_Loaded; }

            /**
             * Returns the callback to pass to the context setup so
             * the internal callbacks are setup correctly below
             */
            v8::DeserializeInternalFieldsCallback GetInternalDeserializerCallback();
            /**
             * Returns the callback to pass to the context setup so
             * the api wrappers are setup correctly below
             */
            v8::DeserializeAPIWrapperCallback GetAPIWrapperDeserializerCallback();
            /** We pass in additional data since we'll need to access the JSContext that creating
             * a deserialized Context before we can attach it
             */
            v8::DeserializeContextDataCallback GetContextDeserializerCallaback(JSContext *inJSContext);

            /**
             * Internal serializers that get the provider and call the real serializer for it
             */

            static void DeserializeInternalField_Internal(V8LObject inHolder, int inIndex, V8StartupData inPayload, void *inData);
            static void DeserializeAPIWrapperField_Internal(V8LObject inHolder, V8StartupData inPayload, void *inData);
            static void DeserializeContextInternalField_Internal(V8LContext inHolder, int inIndex, V8StartupData inPayload, void *inData);

        protected:
            /**
             * Has the data been loaded
             */
            bool m_Loaded = false;
            /**
             * Path of the file the data was loaded from
             */
            std::filesystem::path m_SnapshotPath;
        };
    }
}

#endif //_IJSSNAPSHOT_PROVIDER_H__