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

            JSContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespace, std::filesystem::path inEntryPoint,
                      size_t inContextIndex, std::filesystem::path inSnapEntryPoint = "", bool inSupportsSnapshot = true,
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
             * Reigster the contexts snaphot close handler.
             */
            void RegisterSnapshotCloser();
            /**
             * Unregisters the contexts snapshot closer
             */
            void UnregisterSnapshotCloser();

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
             * Gets the snapshot entry point script. If one hasn't been set
             * then it returns the main entry point script
             */
            std::filesystem::path GetSnapshotEntrypoint() { return (m_SnapEntryPoint.empty()) ? m_EntryPoint : m_SnapEntryPoint; }

            bool RunModule(std::filesystem::path inModulePath);
            V8LValue RunScript(std::string inScript);

            /**
             * Returns whether the context has been initialized
             */
            bool IsInitialized() { return m_Initialized; }
        protected:
            /**
             * Runs the entry point script for the snapshot
             */
            bool RunEntryPoint(bool inSnapshottting);

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
             * Disposes the resources for the js context
             */
            void DisposeContext();

            void MoveContext(JSContext &&inContext);

            /**
             * Teh runtime this context is associated with
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
             * The modules associated with the context
             */
            JSContextModulesSharedPtr m_Modules;
            /**
             * The name of the context
             */
            std::string m_Name;
            /**
             * The snapshot index this context was created from 0 == default v8 context
             */
            size_t m_SnapIndex;
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
             * The entrypoint if snapshotting. If you want to code a specific entrypoint
             * for snapshotting then use this otherwise you can detect in the above
             * script that the context is preparing to be snapshotted
             */
            std::filesystem::path m_SnapEntryPoint;

            /**
             * Indicates if this context is snapshottable. The app may not want all it's created
             * contexts to be snapshooted
             */
            bool m_SupportsSnapshots;
            /**
             * The methof of snapshotting this context
             */
            SnapshotMethod m_SnapMethod;

            JSContext(const JSContext &) = delete;
            JSContext &operator=(const JSContext &) = delete;

            friend class V8ContextProvider;
            //friend JSRuntime;
        };
    }
}

#endif