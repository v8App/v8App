// Copyright 2020 the v8App authors. All right reserved.
// Use of this source code is governed by the MIT license
// that can be found in the LICENSE file.

#include "v8.h"

#include "Logging/LogMacros.h"
#include "Time/Time.h"
#include "Utils/Format.h"

#include "JSRuntime.h"
// #include "JSContext.h"
// #include "JSContextModules.h"
#include "ForegroundTaskRunner.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSRuntime::JSRuntime(IdleTasksSupport inEnableIdle) : m_IdleEnabled(inEnableIdle)
        {
            m_TaskRunner = std::make_shared<ForegroundTaskRunner>();
        }

        JSRuntime::~JSRuntime()
        {
            // clear the contetes before we dispose of the isolate
            {
                v8::Isolate::Scope isolateScope(m_Isolate.get());
                v8::Locker locker(m_Isolate.get());
                v8::HandleScope handleScope(m_Isolate.get());
                m_Contextes.clear();
            }
            JSRuntimeWeakPtr *weakPtr = static_cast<JSRuntimeWeakPtr *>(m_Isolate->GetData(IsolateDataSlot::kJSRuntimePointer));
            delete weakPtr;
            m_Isolate->SetData(IsolateDataSlot::kJSRuntimePointer, nullptr);
            m_Isolate.reset();
        }

        void JSRuntime::CreateIsolate()
        {
            // custom deleter since we have to call dispose
            v8::Isolate *temp = v8::Isolate::Allocate();
            m_Isolate = std::shared_ptr<v8::Isolate>(temp, [](v8::Isolate *isolate)
                                                     { isolate->Dispose(); });
            m_Isolate->SetCaptureStackTraceForUncaughtExceptions(true);
            JSRuntimeWeakPtr *weakPtr = new JSRuntimeWeakPtr(shared_from_this());
            m_Isolate->SetData(IsolateDataSlot::kJSRuntimePointer, weakPtr);

            v8::Isolate::CreateParams params;
            // TODO: replace with custom allocator
            params.array_buffer_allocator =
                v8::ArrayBuffer::Allocator::NewDefaultAllocator();
            v8::Isolate::Initialize(m_Isolate.get(), params);

            //            JSContextModules::SetupModulesCallbacks(m_Isolate.get());
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
            JSRuntimeWeakPtr *weakPtr = static_cast<JSRuntimeWeakPtr *>(inIsloate->GetData(IsolateDataSlot::kJSRuntimePointer));
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

        //        V8ExternalRegistry &JSRuntime::GetExternalRegistry()
        //        {
        //            return m_ExternalRegistry;
        //        }

        void JSRuntime::SetContextCreationHelper(JSContextCreationHelperUniquePtr inCreator)
        {
            m_ContextCreation = std::move(inCreator);
        }
        JSContextWeakPtr JSRuntime::CreateContext(std::string inName)
        {
            DCHECK_NOT_NULL(m_ContextCreation);
            JSContextSharedPtr context = m_ContextCreation->CreateContext(shared_from_this());
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

        JSContextWeakPtr JSRuntime::GetContextByName(std::string inName)
        {
            auto it = m_Contextes.find(inName);
            if (it == m_Contextes.end())
            {
                Log::LogMessage message;
                message.emplace(Log::MsgKey::Msg, Utils::format("Failed to find JSContext with name {}", inName));
                Log::Log::Warn(message);

                return JSContextWeakPtr();
            }
            return JSContextWeakPtr(it->second);
        }

    } // namespace JSRuntime
} // namespace v8App
