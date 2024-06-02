// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_CONTEXT_H_
#define _JS_CONTEXT_H_

#include <unordered_map>

#include "v8/v8.h"

#include "JSRuntime.h"
#include "JSContextModules.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Enum for what type of data is stored in the data slot
         */
        enum class ContextDataSlot : int
        {
            kJSContextWeakPtr = 0
        };

        /**
         * Implments the Context Creation hepler for JSContextes
         */
        class JSContextCreator : public JSContextCreationHelper
        {
        public:
            virtual ~JSContextCreator() = default;
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName) override;
            virtual void DisposeContext(JSContextSharedPtr inContext) override;
            virtual void RegisterSnapshotCloser(JSContextSharedPtr inContext) override;
            virtual void UnregisterSnapshotCloser(JSContextSharedPtr inContext) override;
        };

        /**
         * Class that wraps the v8 context
         */
        class JSContext : public std::enable_shared_from_this<JSContext>, public ISnapshotHandleCloser
        {
        public:
            JSContext(JSRuntimeSharedPtr inRuntime, std::string inName);
            ~JSContext();

            // allow only move constrcutor and assignment
            JSContext(JSContext &&inContext);
            JSContext &operator=(JSContext &&inContext);

            /**
             * Gets the Isolate associated with the context
             */
            v8::Isolate *GetIsolate() { return m_Runtime == nullptr ? nullptr : m_Runtime->GetIsolate(); }
            /**
             * Gets the JSRuntime associated with this context
             */
            JSRuntimeSharedPtr GetJSRuntime() { return m_Runtime; }
            /**
             * Gets the JSContext from the v8 context
             */
            static JSContextSharedPtr GetJSContextFromV8Context(V8LocalContext inContext);

            /**
             * Gets the JSModules for this context
             */
            JSContextModulesSharedPtr GetJSModules() { return m_Modules; }

            /**
             * Gets the name of the context
             */
            std::string GetName() { return m_Name; }

            /**
             * Gets a local context for use
             */
            V8LocalContext GetLocalContext();

            /**
             * Sets up teh callback to create shadow realsm
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

        protected:
            /**
             * Closes teh Global holding the context for snapshots
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
            static V8MaybeLocalContext HostCreateShadowRealmContext(V8LocalContext inInitiator);

            /**
             * Gets the weakref from the v8 context
             */
            JSContextWeakPtr *GetContextWeakRef();

            /**
             * Create the v8 context
             */
            void CreateContext();
            /**
             * Disposes the resources for the js context
             */
            void DisposeContext();

            void MoveContext(JSContext &&inContext);

            JSRuntimeSharedPtr m_Runtime;
            v8::Global<v8::Context> m_Context;
            bool m_Initialized = false;
            JSContextModulesSharedPtr m_Modules;
            std::string m_Name;

            JSContext(const JSContext &) = delete;
            JSContext &operator=(const JSContext &) = delete;

            friend JSContextCreator;
            friend JSRuntime;
        };
    }
}

// template <>
// v8App::JSRuntime::JSContextSharedPtr &v8App::JSRuntime::JSContextSharedPtr::operator=(v8App::JSRuntime::JSContextSharedPtr &&rhs) noexcept;
#endif