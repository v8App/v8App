// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_CONTEXT_H_
#define _JS_CONTEXT_H_

#include <unordered_map>

#include "v8.h"
#include "JSRuntime.h"
#include "JSContextModules.h"

namespace v8App
{
    namespace JSRuntime
    {
        enum class ContextDataSlot : int
        {
            kJSContextWeakPtr = 0
        };

        class JSContextCreator : public JSContextCreationHelper
        {
        public:
            virtual ~JSContextCreator() = default;
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName) override;
            virtual void DisposeContext(JSContextSharedPtr inContext) override;
        };

        class JSContext : public std::enable_shared_from_this<JSContext>
        {
        public:
            JSContext(JSRuntimeSharedPtr inRuntime, std::string inName);
            ~JSContext();

            // allow only move constrcutor and assignment
            JSContext(JSContext &&inContext);
            JSContext &operator=(JSContext &&inContext);

            v8::Isolate *GetIsolate() { return m_Runtime == nullptr ? nullptr : m_Runtime->GetIsolate().get(); }
            JSRuntimeSharedPtr GetJSRuntime() { return m_Runtime; }
            JSContextModulesSharedPtr GetJSModules() { return m_Modules; }
            std::string GetName() { return m_Name; }

            static JSContextSharedPtr GetJSContextFromV8Context(V8LocalContext inContext);

            V8LocalContext GetLocalContext();

            static void SetupShadowRealmCallback(V8Isolate *inIsolate);

        protected:
            std::string GenerateShadowName();
            static V8MaybeLocalContext HostCreateShadowRealmContext(V8LocalContext inInitiator);
            JSContextWeakPtr *GetContextWeakRef();

            void CreateContext();
            void DisposeContext();

            void MoveContext(JSContext &&inContext);

            JSRuntimeSharedPtr m_Runtime;
            V8GlobalContext m_Context;
            bool m_Initialized = false;
            JSContextModulesSharedPtr m_Modules;
            std::string m_Name;

            JSContext(const JSContext &) = delete;
            JSContext &operator=(const JSContext &) = delete;

            friend JSContextCreator;
        };
    }
}

// template <>
// v8App::JSRuntime::JSContextSharedPtr &v8App::JSRuntime::JSContextSharedPtr::operator=(v8App::JSRuntime::JSContextSharedPtr &&rhs) noexcept;
#endif