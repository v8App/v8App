// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <unordered_map>
#include <algorithm>

#include "uuid/uuid.h"

#include "v8/v8.h"
#include "Logging/LogMacros.h"
#include "JSContext.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSContextSharedPtr JSContextCreator::CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName)
        {
            JSContextSharedPtr context = std::make_shared<JSContext>(inRuntime, inName);
            context->CreateContext();
            return context;
        }

        void JSContextCreator::DisposeContext(JSContextSharedPtr inContext)
        {
            inContext->DisposeContext();
        }

        JSContext::JSContext(JSRuntimeSharedPtr inRuntime, std::string inName) : m_Runtime(inRuntime), m_Name(inName)
        {
            CHECK_NE(m_Runtime.get(), nullptr);
            // We use the : a seperator for shadow names
            m_Name.erase(std::remove(m_Name.begin(), m_Name.end(), ':'), m_Name.end());
        }

        JSContext::~JSContext()
        {
            // make sure the context is disposed of
            DisposeContext();
        }

        JSContext::JSContext(JSContext &&inContext)
        {
            MoveContext(std::move(inContext));
        }

        JSContext &JSContext::operator=(JSContext &&inContext)
        {
            MoveContext(std::move(inContext));
            return *this;
        }

        JSContextSharedPtr JSContext::GetJSContextFromV8Context(V8LocalContext inContext)
        {
            JSContextWeakPtr *weakPtr = static_cast<JSContextWeakPtr *>(inContext->GetAlignedPointerFromEmbedderData(int(ContextDataSlot::kJSContextWeakPtr)));
            if (weakPtr == nullptr || weakPtr->expired())
            {
                return JSContextSharedPtr();
            }
            return weakPtr->lock();
        }

        V8LocalContext JSContext::GetLocalContext()
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return V8LocalContext();
            }
            return m_Context.Get(isolate);
        }

        void JSContext::SetupShadowRealmCallback(V8Isolate *inIsolate)
        {
            inIsolate->SetHostCreateShadowRealmContextCallback(JSContext::HostCreateShadowRealmContext);
        }

        std::string JSContext::GenerateShadowName()
        {
            constexpr const char *shadow_str = "shadow";
            constexpr const char *shadow_delim = ":";

            std::size_t delim_pos = m_Name.find_first_of(shadow_delim);
            std::size_t shadow_count = 0;
            std::string name = m_Name.substr(0, delim_pos);

            if (delim_pos != m_Name.npos)
            {
                name = m_Name.substr(0, delim_pos);
                delim_pos = m_Name.find_first_of(shadow_delim, delim_pos + 1);
                if (delim_pos != m_Name.npos)
                {
                    std::string intStrVal = m_Name.substr(delim_pos + 1);
                    shadow_count = std::atoi(intStrVal.c_str());
                }
            }
            shadow_count++;
            return name + shadow_delim + shadow_str + shadow_delim + std::to_string(shadow_count);
        }

        V8MaybeLocalContext JSContext::HostCreateShadowRealmContext(V8LocalContext inInitiator)
        {
            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inInitiator);
            if (jsContext == nullptr)
            {
                return V8MaybeLocalContext();
            }
            std::string shadowName = jsContext->GenerateShadowName();
            JSContextSharedPtr shadow = jsContext->m_Runtime->CreateContext(shadowName);
            V8LocalContext shadow_local = shadow->GetLocalContext();
            shadow_local->SetSecurityToken(inInitiator->GetSecurityToken());

            return jsContext->GetLocalContext();
        }

        void JSContext::MoveContext(JSContext &&inContext)
        {
            // need to get teh wekref first before we move anything
            JSContextWeakPtr *weakRef = inContext.GetContextWeakRef();
            m_Runtime = inContext.m_Runtime;
            inContext.m_Runtime = nullptr;

            m_Initialized = std::move(inContext.m_Initialized);
            inContext.m_Initialized = false;

            m_Context = std::move(inContext.m_Context);
            *weakRef = shared_from_this();
            inContext.m_Context.Reset();

            m_Modules = inContext.m_Modules;
            inContext.m_Modules.reset();
        }

        void JSContext::CreateContext()
        {
            v8::Isolate *isolate = m_Runtime->GetIsolate();
            if (isolate == nullptr)
            {
                return;
            }
            v8::Isolate::Scope isolateScope(isolate);
            v8::TryCatch tryCatch(isolate);
            v8::HandleScope handleScope(isolate);

            // TODO: Hook a setup for classes to register to the global
            v8::Local<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New(isolate);

            V8LocalContext context = v8::Context::New(isolate, nullptr, global_template);
            DCHECK_FALSE(tryCatch.HasCaught());

            if (context.IsEmpty())
            {
                return;
            }
            JSContextWeakPtr *weakPtr = new JSContextWeakPtr(weak_from_this());
            context->SetAlignedPointerInEmbedderData(int(ContextDataSlot::kJSContextWeakPtr), weakPtr);
            m_Modules = std::make_shared<JSContextModules>(shared_from_this());

            // Set the security token for shadow realms.
            uuids::uuid uuid = uuids::uuid_system_generator{}();
            V8LocalString v8Uuid = JSUtilities::StringToV8(isolate, uuids::to_string(uuid));
            context->SetSecurityToken(v8Uuid);

            m_Context.Reset(isolate, context);
            m_Initialized = true;
        }

        void JSContext::DisposeContext()
        {
            if (m_Initialized == false)
            {
                return;
            }
            v8::Isolate *isolate = GetIsolate();
            if (isolate != nullptr)
            {
                if (m_Context.IsEmpty() == false)
                {
                    v8::Local<v8::Context> context = m_Context.Get(isolate);

                    // delink the class pointer
                    JSContextWeakPtr *weakPtr = static_cast<JSContextWeakPtr *>(context->GetAlignedPointerFromEmbedderData(0));
                    if (weakPtr != nullptr)
                    {
                        delete weakPtr;
                    }
                    context->SetAlignedPointerInEmbedderData(int(ContextDataSlot::kJSContextWeakPtr), nullptr);
                }
                m_Context.Reset();
            }
            m_Runtime = nullptr;
            m_Initialized = false;
        }

        JSContextWeakPtr *JSContext::GetContextWeakRef()
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr || m_Context.IsEmpty())
            {
                return nullptr;
            }
            v8::Local<v8::Context> context = m_Context.Get(isolate);
            JSContextWeakPtr *weakPtr = static_cast<JSContextWeakPtr *>(context->GetAlignedPointerFromEmbedderData(int(ContextDataSlot::kJSContextWeakPtr)));
            if (weakPtr == nullptr || weakPtr->expired())
            {
                return nullptr;
            }
            return weakPtr;
        }
    }
}

// template <>
// v8App::JSRuntime::JSContextSharedPtr &v8App::JSRuntime::JSContextSharedPtr::operator=(v8App::JSRuntime::JSContextSharedPtr &&rhs) noexcept
//{
//     (*this)->MoveContext(std::move(*rhs));
//     (*this)->SetContextWeakRef(*this);
//     return *this;
// }
