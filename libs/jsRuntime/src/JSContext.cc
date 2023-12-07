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
        JSContext::JSContext(JSRuntimeSharedPtr inIsolate) : m_Isolate(inIsolate)
        {
        }

        JSContext::~JSContext()
        {
            // make sure the context is disposed of
            DisposeContext();
        }

        // allow only move constrcutor and assignment
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

            //m_Modules = inContext.m_Modules;
            //inContext.m_Modules.reset();
        }

        bool JSContext::InitializeContext(JSContextSharedPtr inSharedContext)
        {
            if (m_Initialized)
            {
                return true;
            }
            if (m_Isolate == nullptr)
            {
                return false;
            }
            v8::Isolate *isolate = m_Isolate->GetIsolate().get();
            if (isolate == nullptr)
            {
                return false;
            }
            //m_Modules = std::make_shared<JSContextModules>(inSharedContext);
            
            v8::TryCatch tryCatch(isolate);

            // TODO: Hook a setup for classes to register to the global
            v8::Local<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New(isolate);

            v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr, global_template);
            DCHECK_FALSE(tryCatch.HasCaught());

            if (context.IsEmpty())
            {
                return false;
            }
            SetContextWeakRef(inSharedContext);
            m_Context.Reset(isolate, context);
            m_Initialized = true;
            return true;
        }

        void JSContext::DisposeContext()
        {
            if (m_Initialized == false)
            {
                return;
            }
            if (m_Context.IsEmpty() == false)
            {
                v8::Isolate *isolate = GetIsolate();
                if (isolate != nullptr)
                {
                    v8::Local<v8::Context> context = m_Context.Get(isolate);

                    // delink the class pointer
                    JSContextWeakPtr *weakPtr = static_cast<JSContextWeakPtr *>(context->GetAlignedPointerFromEmbedderData(0));
                    if (weakPtr != nullptr)
                    {
                        delete weakPtr;
                    }
                    context->SetAlignedPointerInEmbedderData(0, nullptr);
                }
            }
            m_Context.Reset();

            m_Isolate = nullptr;

            m_Initialized = false;
        }

            v8::Isolate *JSContext::GetIsolate()  
            { 
                return m_Isolate == nullptr? nullptr : m_Isolate->GetIsolate().get(); 
            }

        JSContextSharedPtr JSContext::GetJSContextFromV8Context(v8::Local<v8::Context> inContext)
        {
            JSContextWeakPtr *weakPtr = static_cast<JSContextWeakPtr *>(inContext->GetAlignedPointerFromEmbedderData(0));
            if (weakPtr == nullptr || weakPtr->expired())
            {
                return JSContextSharedPtr();
            }
            return weakPtr->lock();
        }

        void JSContext::SetContextWeakRef(JSContextSharedPtr inContext)
        {
            JSContextWeakPtr *weakPtr = GetContextWeakRef(inContext);
            if (weakPtr != nullptr)
            {
                delete weakPtr;
            }
            if (inContext->GetIsolate() == nullptr)
            {
                return;
            }
            weakPtr = new JSContextWeakPtr(inContext);

            m_Context.Get(inContext->GetIsolate())->SetAlignedPointerInEmbedderData(0, weakPtr);
        }

        JSContextWeakPtr *JSContext::GetContextWeakRef(JSContextSharedPtr inContext)
        {
            v8::Isolate *isolate = inContext->GetIsolate();
            if (isolate == nullptr || inContext->m_Context.IsEmpty())
            {
                return nullptr;
            }
            v8::Local<v8::Context> context = inContext->m_Context.Get(isolate);
            JSContextWeakPtr *weakPtr = static_cast<JSContextWeakPtr *>(context->GetAlignedPointerFromEmbedderData(0));
            if (weakPtr == nullptr || weakPtr->expired())
            {
                return nullptr;
            }
            return weakPtr;
        }
    }
}

template <>
v8App::JSRuntime::JSContextSharedPtr &v8App::JSRuntime::JSContextSharedPtr::operator=(v8App::JSRuntime::JSContextSharedPtr &&rhs) noexcept
{
    (*this)->MoveContext(std::move(*rhs));
    (*this)->SetContextWeakRef(*this);
    return *this;
}
