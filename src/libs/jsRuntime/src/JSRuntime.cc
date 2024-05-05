// Copyright 2020 the v8App authors. All right reserved.
// Use of this source code is governed by the MIT license
// that can be found in the LICENSE file.

#include "v8/v8.h"

#include "Logging/LogMacros.h"
#include "Time/Time.h"
#include "Utils/Format.h"

#include "JSRuntime.h"
#include "JSContextModules.h"
#include "ForegroundTaskRunner.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSRuntime::JSRuntime(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName)
            : m_IdleEnabled(inEnableIdle), m_App(inApp), m_Name(inName)
        {
            m_TaskRunner = std::make_shared<ForegroundTaskRunner>();
        }

        JSRuntime::~JSRuntime()
        {
            DisposeRuntime();
        }

        JSRuntimeSharedPtr JSRuntime::CreateJSRuntime(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName,
                                                      const v8::StartupData *inSnapshot, const intptr_t *inExternalReferences)
        {
            JSRuntimeSharedPtr temp = std::make_shared<JSRuntime>(inApp, inEnableIdle, inName);
            temp->CreateIsolate();

            v8::Isolate::CreateParams params;
            params.snapshot_blob = inSnapshot;
            params.external_references = inExternalReferences;
            // TODO: replace with custom allocator
            params.array_buffer_allocator =
                v8::ArrayBuffer::Allocator::NewDefaultAllocator();
            v8::Isolate::Initialize(temp->m_Isolate.get(), params);

            return temp;
        }

        JSRuntimeSharedPtr JSRuntime::CreateJSRuntimeForSnapshot(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName)
        {
            JSRuntimeSharedPtr temp = std::make_shared<JSRuntime>(inApp, inEnableIdle, inName);
            temp->CreateIsolate();

            return temp;
        }

        void JSRuntime::CreateIsolate()
        {
            // custom deleter since we have to call dispose
            v8::Isolate *temp = v8::Isolate::Allocate();
            m_Isolate = std::shared_ptr<v8::Isolate>(temp, [](v8::Isolate *isolate)
                                                     {isolate->Dispose(); });
            m_Isolate->SetCaptureStackTraceForUncaughtExceptions(true);

            JSRuntimeWeakPtr *weakPtr = new JSRuntimeWeakPtr(shared_from_this());
            m_Isolate->SetData(uint32_t(IsolateDataSlot::kJSRuntimeWeakPtr), weakPtr);
        }

        void JSRuntime::Initialize()
        {
            JSContextModules::SetupModulesCallbacks(m_Isolate.get());
        }

        V8TaskRunnerSharedPtr JSRuntime::GetForegroundTaskRunner()
        {
            return m_TaskRunner;
        }

        bool JSRuntime::IdleTasksEnabled()
        {
            return m_IdleEnabled == IdleTasksSupport::kIdleTasksEnabled;
        }

        JSRuntimeSharedPtr JSRuntime::GetJSRuntimeFromV8Isolate(v8::Isolate *inIsloate)
        {
            if (inIsloate == nullptr)
            {
                return nullptr;
            }
            JSRuntimeWeakPtr *weakPtr = static_cast<JSRuntimeWeakPtr *>(inIsloate->GetData(uint32_t(IsolateDataSlot::kJSRuntimeWeakPtr)));
            if (weakPtr == nullptr || weakPtr->expired())
            {
                return JSRuntimeSharedPtr();
            }
            return weakPtr->lock();
        }

        void JSRuntime::ProcessTasks()
        {
            while (m_TaskRunner->MaybeHasTask())
            {
                V8TaskUniquePtr task = m_TaskRunner->GetNextTask();
                {
                    v8::Isolate::Scope isolateScope(m_Isolate.get());
                    v8::Locker locker(m_Isolate.get());
                    ForegroundTaskRunner::TaskRunScope runScope(m_TaskRunner);
                    task->Run();
                }
            }
        }

        void JSRuntime::ProcessIdleTasks(double inTimeLeft)
        {
            if (IdleTasksEnabled() == false)
            {
                return;
            }

            double deadline = Time::MonotonicallyIncreasingTimeSeconds() + inTimeLeft;

            while (deadline > Time::MonotonicallyIncreasingTimeSeconds() && m_TaskRunner->MaybeHasIdleTask())
            {
                V8IdleTaskUniquePtr task = m_TaskRunner->GetNextIdleTask();
                {
                    v8::Isolate::Scope isolateScope(m_Isolate.get());
                    v8::Locker locker(m_Isolate.get());
                    ForegroundTaskRunner::TaskRunScope runScope(m_TaskRunner);
                    task->Run(deadline);
                }
            }
        }

        void JSRuntime::SetObjectTemplate(void *inInfo, v8::Local<v8::ObjectTemplate> inTemplate)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            m_ObjectTemplates[inInfo] = v8::Eternal<v8::ObjectTemplate>(m_Isolate.get(), inTemplate);
        }

        v8::Local<v8::ObjectTemplate> JSRuntime::GetObjectTemplate(void *inInfo)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            ObjectTemplateMap::iterator it = m_ObjectTemplates.find(inInfo);
            if (it == m_ObjectTemplates.end())
            {
                return v8::Local<v8::ObjectTemplate>();
            }
            return it->second.Get(m_Isolate.get());
        }

        void JSRuntime::SetFunctionTemplate(void *inInfo, v8::Local<v8::FunctionTemplate> inTemplate)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            m_FunctionTemplates[inInfo] = v8::Eternal<v8::FunctionTemplate>(m_Isolate.get(), inTemplate);
        }

        v8::Local<v8::FunctionTemplate> JSRuntime::GetFunctionTemplate(void *inInfo)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            FunctionTemplateMap::iterator it = m_FunctionTemplates.find(inInfo);
            if (it == m_FunctionTemplates.end())
            {
                return v8::Local<v8::FunctionTemplate>();
            }
            return it->second.Get(m_Isolate.get());
        }

        void JSRuntime::SetContextCreationHelper(JSContextCreationHelperUniquePtr inCreator)
        {
            m_ContextCreation = std::move(inCreator);
        }
        JSContextSharedPtr JSRuntime::CreateContext(std::string inName)
        {
            CHECK_NOT_NULL(m_ContextCreation);
            JSContextSharedPtr context = m_ContextCreation->CreateContext(shared_from_this(), inName);
            if (context == nullptr)
            {
                Log::LogMessage message;
                message.emplace(Log::MsgKey::Msg, "ContextHelper returned a nullptr for the context");
                Log::Log::Error(message);
            }
            else
            {
                m_Contextes.insert(std::make_pair(inName, context));
            }
            return context;
        }

        JSContextSharedPtr JSRuntime::GetContextByName(std::string inName)
        {
            auto it = m_Contextes.find(inName);
            if (it == m_Contextes.end())
            {
                Log::LogMessage message;
                message.emplace(Log::MsgKey::Msg, Utils::format("Failed to find JSContext with name {}", inName));
                Log::Log::Warn(message);

                return JSContextSharedPtr();
            }
            return it->second;
        }

        void JSRuntime::DisposeContext(std::string inName)
        {
            JSContextSharedPtr context = GetContextByName(inName);
            DisposeContext(context);
        }

        void JSRuntime::DisposeContext(JSContextSharedPtr inContext)
        {
            DCHECK_NOT_NULL(m_ContextCreation);
            if (inContext == nullptr)
            {
                return;
            }
            m_ContextCreation->DisposeContext(inContext);
            for (auto it = m_Contextes.begin(); it != m_Contextes.end(); it++)
            {
                if (it->second == inContext)
                {
                    m_Contextes.erase(it);
                    return;
                }
            }
        }

        void JSRuntime::DisposeRuntime()
        {
            if (m_Isolate != nullptr)
            {
                v8::Isolate::Scope isolateScope(m_Isolate.get());
                v8::Locker locker(m_Isolate.get());
                v8::HandleScope handleScope(m_Isolate.get());
                m_Contextes.clear();
                JSRuntimeWeakPtr *weakPtr = static_cast<JSRuntimeWeakPtr *>(m_Isolate->GetData(uint32_t(IsolateDataSlot::kJSRuntimeWeakPtr)));
                delete weakPtr;
                m_Isolate->SetData(uint32_t(IsolateDataSlot::kJSRuntimeWeakPtr), nullptr);
            }
            m_Isolate.reset();
            m_App.reset();
            m_TaskRunner.reset();

            // TODO: Adds something to delete the object.function templates
        }

    } // namespace JSRuntime
} // namespace v8App
