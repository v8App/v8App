// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_RUNTIME_H_
#define _JS_RUNTIME_H_

#include <memory>
#include<map>

#include "v8-platform.h"
#include "v8.h"
#include "V8ExternalRegistry.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TaskRunner;
        class DelayedWorkerTaskQueue;

        using TimeFunction = std::function<double()>;
        using TaskPtr = std::unique_ptr<v8::Task>;
        using IdleTaskPtr = std::unique_ptr<v8::IdleTask>;
        using JSRuntimeWeakPtr = std::weak_ptr<class JSRuntime>;
        using JSRuntimeSharedPtr = std::shared_ptr<class JSRuntime>;
        using IsolateWeakPtr = std::weak_ptr<v8::Isolate>;

        enum IsolateDataSlot
        {
            kJSRuntimePointer
        };

        enum class IdleTasksSupport
        {
            kIdleTasksDisabled,
            kIdleTasksEnabled
        };

        class JSRuntime
        {
        public:
            explicit JSRuntime(IdleTasksSupport inEnableIdle);
            virtual ~JSRuntime();

            virtual std::shared_ptr<v8::TaskRunner> GetForegroundTaskRunner();
            virtual bool AreIdleTasksEnabled();

            static JSRuntime *GetRuntime(v8::Isolate *inIsloate);
            std::weak_ptr<v8::Isolate> GetIsolate() { return m_Isolate; }

            void ProcessTasks();
            void ProcessIdleTasks(double inTimeLeft);

            void SetObjectTemplate(void *inInfo, v8::Local<v8::ObjectTemplate> inTemplate);
            v8::Local<v8::ObjectTemplate> GetObjectTemplate(void *inInfo);

            void SetFunctionTemplate(void *inInfo, v8::Local<v8::FunctionTemplate> inTemplate);
            v8::Local<v8::FunctionTemplate> GetFunctionTemplate(void *inInfo);

            V8ExternalRegistry& GetExternalRegistry();

        protected:
            IdleTasksSupport m_IdleEnabled;

            //this has a custom no op deleter since we can't call delete on the pointer.
            //we do this so in our code we can use weak ptrs as protection agaisnt tasks using the isolate after
            //deletion
            std::shared_ptr<v8::Isolate> m_Isolate;
            //the isolate's task queues
            std::shared_ptr<TaskRunner> m_TaskRunner;
            std::unique_ptr<DelayedWorkerTaskQueue> m_DelayedWorkerTasks;

            using ObjectTemplateMap = std::map<void *, v8::Eternal<v8::ObjectTemplate>>;
            using FunctionTemplateMap = std::map<void *, v8::Eternal<v8::FunctionTemplate>>;

            ObjectTemplateMap m_ObjectTemplates;
            FunctionTemplateMap m_FunctionTemplates;

            V8ExternalRegistry m_ExternalRegistry;

            JSRuntime(const JSRuntime &) = delete;
            JSRuntime &operator=(const JSRuntime &) = delete;
        };
    } // namespace JSRuntime
} // namespace v8App
#endif
