// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_CONTEXT_H_
#define _JS_CONTEXT_H_

#include <unordered_map>
#include <filesystem>

#include "JSRuntime.h"
#include "IJSContextProvider.h"
#include "JSContextModules.h"
#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Class that wraps the v8 context
         */
        class JSContext : public std::enable_shared_from_this<JSContext>, public ISnapshotHandleCloser
        {
        public:
            /**
             * Enum for what type of data is stored in the data slot
             */
            enum class DataSlot : int
            {
                kJSContextWeakPtr = 0
            };
            JSContext() {}

            JSContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespace, std::filesystem::path inEntryPoint,
                      size_t inContextIndex, bool inUseV8Default, bool inSupportsSnapshot = true,
                      SnapshotMethod inSnapMethod = SnapshotMethod::kNamespaceOnly);
            ~JSContext();

            // allow only move constrcutor and assignment
            JSContext(JSContext &&inContext);
            JSContext &operator=(JSContext &&inContext);

            /**
             * Gets the Isolate associated with the context
             */
            V8Isolate *GetIsolate();
            /**
             * Gets the JSRuntime associated with this context
             */
            JSRuntimeSharedPtr GetJSRuntime() { return m_Runtime; }

            /**
             * Shortcut to get to the JSApp faster than chaining through the GetJSRuntime() above
             */
            JSAppSharedPtr GetJSApp();

            /**
             * Gets the JSContext from the v8 context
             */
            static JSContextSharedPtr GetJSContextFromV8Context(V8LContext inContext);

            /**
             * Gets the JSModules for this context
             */
            JSContextModulesSharedPtr GetJSModules() { return m_Modules; }

            /**
             * Gets the name of the context
             */
            std::string GetName() { return m_Name; }
            /**
             * Gets the contexts namespace. no namespace means a default v8 context
             */
            std::string GetNamespace() { return m_Namespace; }
            /**
             * Gets the snapshot index context created from
             */
            size_t GetSnapshotIndex() { return m_SnapIndex; }

            /**
             * Gets a local context for use
             */
            V8LContext GetLocalContext();

            /**
             * Sets up the callback to create shadow realsm
             */
            static void SetupShadowRealmCallback(V8Isolate *inIsolate);

            /**
             * Returns if the context supports being snapshotted
             */
            bool SupportsSnapshots() { return m_SupportsSnapshots; }

            /**
             * Gets the method fo snapshotting the context
             */
            SnapshotMethod GetSnapshotMethod() { return m_SnapMethod; }

            /**
             * Gets the entry point script
             */
            std::filesystem::path GetEntrypoint() { return m_EntryPoint; }

            /**
             * Run the specified file as a module
             */
            V8LValue RunModule(std::filesystem::path inModulePath);
            /**
             * Runs the specified string as js code
             */
            V8LValue RunScript(std::string inScript);

            /**
             * Returns whether the context has been initialized
             */
            bool IsInitialized() { return m_Initialized; }

            /**
             * Gets the security token set for the context
             */
            std::string GetSecurityToken() { return m_SecurityToken; }

            /**
             * Serializes the context data internal field data
             */
            void SerializeContextData(Serialization::WriteBuffer &inBuffer);
            /**
             * Deserializes the context's internal field data
             */
            void DeserializeContextData(V8LContext inContext, Serialization::ReadBuffer &inBuffer);

            /**
             * Make a snapshot of the context
             */
            bool MakeSnapshot(V8SnapshotCreatorSharedPtr inCreator, v8App::Serialization::WriteBuffer &inBuffer);
            /**
             * Loads the context's snapshot data
             */
            JSContextSnapDataSharedPtr LoadSnapshotData(v8App::Serialization::ReadBuffer &inBuffer);

            /**
             * Subclasses should override and return their snap data object if they have other data they
             * need snapshotted
             */
            virtual JSContextSnapDataSharedPtr CreateSnapData() { return std::make_shared<JSContextSnapData>(); }

        protected:
            /**
             * Runs the entry point script for the snapshot
             */
            V8LValue RunEntryPoint(bool inSnapshottting);

            /**
             * Clones the context for snapshotting using the specified runtime
             */
            JSContextSharedPtr CloneForSnapshot(JSRuntimeSharedPtr inRuntime);

            /**
             * Closes the Global holding the context for snapshots
             */
            virtual void CloseHandleForSnapshot() override;

            /**
             * Closes out the Global for the context and removes the weak ref so a
             *  snapshot can be generated
             */
            void DisposeV8Context(bool forSnapshot = false);

            /**
             * Generates a context for the shadow realm in the format of
             * <base context name>:shadow:<int>
             */
            std::string GenerateShadowName();
            /**
             * Callback V8 calls to create the shadow realm
             */
            static V8MBLContext HostCreateShadowRealmContext(V8LContext inInitiator);

            /**
             * Gets the weakref from the v8 context
             */
            JSContextWeakPtr *GetContextWeakRef();

            /**
             * Create the v8 context from the given snapshot index.
             */
            bool CreateContext();

            /**
             * creates and attaches the weak ptr to the context
             */
            void AttachWeakPtr(V8LContext inContext);

            /**
             * Disposes the resources for the js context
             */
            void DisposeContext();

            /**
             * Moves the context to a new instance
             */
            void MoveContext(JSContext &&inContext);

            /**
             * Snapshot data
             * ******************************************
             */
            /**
             * The modules associated with the context
             */
            JSContextModulesSharedPtr m_Modules;

            /**
             * The name of the context
             */
            std::string m_Name;

            /**
             * The namespace tha tthe context was created with.
             * No namespace means it on;y has the default v8 builtins
             */
            std::string m_Namespace;

            /**
             * The entry point script for the context
             */
            std::filesystem::path m_EntryPoint;

            /**
             * Non Snapshot data
             * ******************************************
             */

            /**
             * The snapshot index this context was created from 0 == default v8 context
             */
            size_t m_SnapIndex;

            /**
             * The runtime this context is associated with
             */
            JSRuntimeSharedPtr m_Runtime;
            /**
             * The actual v8 context
             */
            V8GContext m_Context;
            /**
             * Has the context been initialized yet
             */
            bool m_Initialized{false};

            /**
             * Indicates if this context is snapshottable. The app may not want all it's created
             * contexts to be snapshooted
             */
            bool m_SupportsSnapshots;
            /**
             * The methof of snapshotting this context
             */
            SnapshotMethod m_SnapMethod;

            /**
             * The generated Security token for the context
             */
            std::string m_SecurityToken;

            /**
             * Whether the context was the v8 default one
             */
            bool m_V8Default{false};

            JSContext(const JSContext &) = delete;
            JSContext &operator=(const JSContext &) = delete;

            friend class V8ContextProvider;
            friend JSRuntime;
        };
    }
}

#endif