// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_CONTEXT_H_
#define _JS_CONTEXT_H_

#include <unordered_map>

#include "v8.h"
#include "JSModules.h"

namespace v8App
{
    namespace JSRuntime
    {
        using SharedJSContextPtr = std::shared_ptr<class JSContext>;
        using WeakJSContextPtr = std::weak_ptr<class JSContext>;

        class JSContext
        {
            public:
            JSContext(v8::Isolate* inIsolate);
            ~JSContext();

            //allow only move constrcutor and assignment
            JSContext(JSContext&& inContext);
            JSContext& operator=(JSContext&& inContext);
            
            bool InitializeContext();
            void DisposeContext();

            v8::Local<v8::Context> GetContext() { return m_Context.Get(m_Isolate); }
            v8::Isolate* GetIsolate() const { return m_Isolate; }

            WeakJSModulesPtr GetModules() { return m_Modules; }


            static JSContext* GetJSContext(v8::Local<v8::Context> inContext);

            private:
            void MoveContext(JSContext&& inContext);

            v8::Isolate* m_Isolate;
            v8::Global<v8::Context> m_Context;
            bool m_Initialized = false;

            SharedJSModulesPtr m_Modules;

            JSContext(const JSContext&) = delete;
            JSContext& operator=(const JSContext&) = delete;
        };
    }
}

#endif