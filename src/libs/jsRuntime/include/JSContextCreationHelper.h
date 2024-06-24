// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_CONTEXT_CREATION_HELPER_H__
#define _JS_CONTEXT_CREATION_HELPER_H__

#include <map>
#include <filesystem>

#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        /*
         * Used to isolate the context creation during testing
         */
        class JSContextCreationHelper
        {
        public:
            virtual ~JSContextCreationHelper() = default;
            /**
             * Override to create a v8 context
             */
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespce,
            std::filesystem::path inEntryPoint, std::filesystem::path inSnapEntryPoint, bool inSupportsSnapshot, 
                      SnapshotMethod inSnapMethod) = 0;
            /**
             * Override to dispose of the v8 context
             */
            virtual void DisposeContext(JSContextSharedPtr inContext) = 0;
            virtual void RegisterSnapshotCloser(JSContextSharedPtr inContext) = 0;
            virtual void UnregisterSnapshotCloser(JSContextSharedPtr inContext) = 0;

            /**
             * Gets the namesapce for the associated snapshot index.
             * An empty namespace will default the dfault v8 context at 0
             */
            std::string GetNamespaceForSnapIndex(size_t inSnapIndex);
            /**
             * Gets the snapshot index for the given namespace
             */
            size_t GetSnapIndexForNamespace(std::string inNamespace);

        protected:
            /**
             * Serializes the snapshot namespace info for a snapshot
             */
            bool SetContextNamespaces(v8::StartupData *inData);
            /**
             * Desrializes the snapshot namespaces from a snapshot
             */
            v8::StartupData SerializeContextNamespaces();
            /**
             * Addes the snapshot index for the namespace
             */
            bool AddSnapIndexNamespace(size_t inSnapIndex, std::string inNamespace);

            std::map<int, std::string> m_NamespacesSnapIndexes;
            friend class JSRuntime;
        };

    }
}
#endif //_JS_CONTEXT_CREATION_HELPER_H__