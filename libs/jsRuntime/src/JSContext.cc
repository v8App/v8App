// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "v8.h"
#include "Logging/LogMacros.h"
#include "JSContext.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSContext::JSContext(v8::Isolate *inIsolate) : m_Isolate(inIsolate)
        {
            m_Modules = std::make_shared<JSModules>(this, inIsolate);
        }

        JSContext::~JSContext()
        {
            //make sure the context is disposed of
            DisposeContext();
        }

        //allow only move constrcutor and assignment
        JSContext::JSContext(JSContext &&inContext)
        {
            MoveContext(std::move(inContext));
        }

        JSContext &JSContext::operator=(JSContext &&inContext)
        {
            MoveContext(std::move(inContext));
            return *this;
        }

        void JSContext::MoveContext(JSContext &&inContext)
        {
            m_Isolate = inContext.m_Isolate;
            inContext.m_Isolate = nullptr;

            m_Initialized = inContext.m_Initialized;
            inContext.m_Initialized = false;

            m_Context = std::move(inContext.m_Context);
            inContext.m_Context.Reset();

            m_Modules = std::move(inContext.m_Modules);
            //reset the context's pointer to the object
            m_Context.Get(m_Isolate)->SetAlignedPointerInEmbedderData(0, this);
        }

        bool JSContext::InitializeContext()
        {
            if (m_Initialized)
            {
                return true;
            }

            v8::TryCatch tryCatch(m_Isolate);

            //TODO: Hook a setup for classes to register to the global
            v8::Local<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New(m_Isolate);

            v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, global_template);
            DCHECK_FALSE(tryCatch.HasCaught());

            if (context.IsEmpty())
            {
                return false;
            }

            context->SetAlignedPointerInEmbedderData(0, this);
            m_Context.Reset(m_Isolate, context);
            m_Initialized = true;
            return true;
        }

        void JSContext::DisposeContext()
        {
            if (m_Initialized == false)
            {
                return;
            }
            //delink the class pointer
            if (m_Context.IsEmpty() == false)
            {
                m_Context.Get(m_Isolate)->SetAlignedPointerInEmbedderData(0, nullptr);
                m_Context.Reset();
            }
            m_Isolate = nullptr;
            m_Modules = nullptr;

            m_Initialized = false;
        }

        JSContext *JSContext::GetJSContext(v8::Local<v8::Context> inContext)
        {
            return static_cast<JSContext *>(inContext->GetAlignedPointerFromEmbedderData(0));
        }

    }
}
