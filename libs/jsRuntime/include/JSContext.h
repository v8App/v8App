// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_CONTEXT_H_
#define _JS_CONTEXT_H_

#include <unordered_map>

#include "v8.h"
#include "JSRuntime.h"
//#include "JSContextModules.h"

namespace v8App
{
    namespace JSRuntime
    {

        class JSContext
        {
        public:
            JSContext(JSRuntimeSharedPtr inIsolate);
            ~JSContext();

            // allow only move constrcutor and assignment
            JSContext(JSContext &&inContext);
            JSContext &operator=(JSContext &&inContext);

            bool InitializeContext(JSContextSharedPtr inContext);
            void DisposeContext();

            v8::Local<v8::Context> GetContext()
            {
                v8::Isolate *isolate = GetIsolate();
                if (isolate == nullptr)
                {
                    return v8::Local<v8::Context>();
                }
                return m_Context.Get(isolate);
            }
            v8::Isolate *GetIsolate();
            JSRuntimeSharedPtr GetJSRuntime() { return m_Isolate; }
            //JSContextModulesSharedPtr GetJSModules() { return m_Modules; }

            static JSContextSharedPtr GetJSContextFromV8Context(v8::Local<v8::Context> inContext);

        protected:
            void SetContextWeakRef(JSContextSharedPtr inContext);
            JSContextWeakPtr *GetContextWeakRef(JSContextSharedPtr inContext);

        private:
            void MoveContext(JSContext &&inContext);

            JSRuntimeSharedPtr m_Isolate;
            v8::Global<v8::Context> m_Context;
            bool m_Initialized = false;
            //JSContextModulesSharedPtr m_Modules;

            JSContext(const JSContext &) = delete;
            JSContext &operator=(const JSContext &) = delete;

            friend JSContextSharedPtr;
        };
    }
}

template <>
v8App::JSRuntime::JSContextSharedPtr &v8App::JSRuntime::JSContextSharedPtr::operator=(v8App::JSRuntime::JSContextSharedPtr &&rhs) noexcept;
#endif