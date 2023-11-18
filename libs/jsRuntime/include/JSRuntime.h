// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_RUNTIME_H_
#define _JS_RUNTIME_H_

#include <memory>
#include <map>

#include "Assets/AppAssetRoots.h"
#include "V8ExternalRegistry.h"

#include "v8-platform.h"
#include "v8.h"

#ifdef UNIT_TESTING
#include <gtest/gtest_prod.h>
#endif

namespace v8App
{
    namespace JSRuntime
    {
        class TaskRunner;
        class DelayedWorkerTaskQueue;
        class JSContext;

        using TimeFunction = std::function<double()>;
        using TaskPtr = std::unique_ptr<v8::Task>;
        using IdleTaskPtr = std::unique_ptr<v8::IdleTask>;
        using JSRuntimeWeakPtr = std::weak_ptr<class JSRuntime>;
        using JSRuntimeSharedPtr = std::shared_ptr<class JSRuntime>;
        using IsolateSharedPtr = std::shared_ptr<v8::Isolate>;
        using IsolateWeakPtr = std::weak_ptr<v8::Isolate>;
        using JSContextSharedPtr = std::shared_ptr<class JSContext>;
        using JSContextWeakPtr = std::weak_ptr<class JSContext>;
        using V8TaskRunnerSharedPtr = std::shared_ptr<v8::TaskRunner>;

        enum IsolateDataSlot
        {
            kJSRuntimePointer
        };

        enum class IdleTasksSupport
        {
            kIdleTasksDisabled,
            kIdleTasksEnabled
        };

        class JSRuntime : std::enable_shared_from_this<JSRuntime>
        {
        public:
            explicit JSRuntime(IdleTasksSupport inEnableIdle);
            JSRuntime(JSRuntime&&) = default;

            static JSRuntimeSharedPtr CreateJSRuntime(IdleTasksSupport inEnableIdle)
            {
                JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>(inEnableIdle);
                runtime->SetIsolateWeakRef(runtime);
                return runtime;
            }
            virtual ~JSRuntime();

            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner();
            virtual bool AreIdleTasksEnabled();

            static JSRuntimeSharedPtr GetJSRuntimeFromV8Isolate(v8::Isolate *inIsloate);
            const IsolateSharedPtr GetIsolate() { return m_Isolate; }

            void ProcessTasks();
            void ProcessIdleTasks(double inTimeLeft);

            void SetObjectTemplate(void *inInfo, v8::Local<v8::ObjectTemplate> inTemplate);
            v8::Local<v8::ObjectTemplate> GetObjectTemplate(void *inInfo);

            void SetFunctionTemplate(void *inInfo, v8::Local<v8::FunctionTemplate> inTemplate);
            v8::Local<v8::FunctionTemplate> GetFunctionTemplate(void *inInfo);

            V8ExternalRegistry &GetExternalRegistry();

            JSContextWeakPtr CreateContext(std::string inName);
            JSContextWeakPtr GetContextByName(std::string inName);

            void SetAppAssetRoots(Assets::AppAssetRootsSharedPtr inRoots) { m_AppRoots = inRoots; }
            const Assets::AppAssetRootsSharedPtr &GetAppRoots() const { return m_AppRoots; }

        protected:
            void SetIsolateWeakRef(JSRuntimeSharedPtr runtime);

            IdleTasksSupport m_IdleEnabled;

            // this has a custom no op deleter since we can't call delete on the pointer.
            // we do this so in our code we can use weak ptrs as protection agaisnt tasks using the isolate after
            // deletion
            IsolateSharedPtr m_Isolate;
            std::map<std::string, JSContextSharedPtr> m_Contextes;

            // the isolate's task queues
            std::shared_ptr<TaskRunner> m_TaskRunner;
            std::unique_ptr<DelayedWorkerTaskQueue> m_DelayedWorkerTasks;

            using ObjectTemplateMap = std::map<void *, v8::Eternal<v8::ObjectTemplate>>;
            using FunctionTemplateMap = std::map<void *, v8::Eternal<v8::FunctionTemplate>>;

            ObjectTemplateMap m_ObjectTemplates;
            FunctionTemplateMap m_FunctionTemplates;

            V8ExternalRegistry m_ExternalRegistry;

            Assets::AppAssetRootsSharedPtr m_AppRoots;

            JSRuntime(const JSRuntime &) = delete;
            JSRuntime &operator=(const JSRuntime &) = delete;

#ifdef UNIT_TESTING
            FRIEND_TEST(JSRuntimeTest, ConstrcutorRelated);
#endif
        };
    } // namespace JSRuntime
} // namespace v8App
#endif
