// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_RUNTIME_H_
#define _JS_RUNTIME_H_

#include <memory>
#include <map>

#include "Utils/Format.h"

#include "JSApp.h"
#include "V8Types.h"
#include "V8Platform.h"
#include "JSSnapshotCreator.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSContext;

        enum class IsolateDataSlot : uint32_t
        {
            kJSRuntimeWeakPtr = 0
        };

        enum class IdleTasksSupport
        {
            kIdleTasksDisabled,
            kIdleTasksEnabled
        };

        class JSContextCreationHelper
        {
        public:
            virtual ~JSContextCreationHelper() = default;
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName) = 0;
            virtual void DisposeContext(JSContextSharedPtr inContext) = 0;
        };

        using JSContextCreationHelperUniquePtr = std::unique_ptr<JSContextCreationHelper>;

        inline std::string GenerateRuntimeName(std::string inFile, std::string inFunc, int inLine)
        {
            return Utils::format("JSRuntime_{}_{}_{}", inFile, inFunc, inLine);
        }

#define GENERATE_JSRUNTIME_NAME() GenerateRuntimeName(__FILE__, __func__, __LINE__)

        class JSRuntime : public std::enable_shared_from_this<JSRuntime>
        {
        public:
            explicit JSRuntime(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName);
            virtual ~JSRuntime();

            JSRuntime(JSRuntime &&) = default; // TODO: use non default cause of the wekref

            /**
             * Creates a JSRuntime and V8 isolate and inializes the iisolate for use.
             */
            static JSRuntimeSharedPtr CreateJSRuntime(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName,
                                                      const v8::StartupData *inSnapshot = nullptr, const intptr_t *inExternalReferences = nullptr);

            /**
             * Creaates a JSRuntime with a V8 Isolate but does not initlize the isolate. SnapshotCreator will init the isolate
             */
            static JSRuntimeSharedPtr CreateJSRuntimeForSnapshot(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName);

            void Initialize();

            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner();
            virtual bool IdleTasksEnabled();

            static JSRuntimeSharedPtr GetJSRuntimeFromV8Isolate(v8::Isolate *inIsloate);
            const V8IsolateSharedPtr GetSharedIsolate() { return m_Isolate; }
            V8Isolate *GetIsolate() { return m_Isolate.get(); }

            void ProcessTasks();
            void ProcessIdleTasks(double inTimeLeft);

            void SetObjectTemplate(void *inInfo, v8::Local<v8::ObjectTemplate> inTemplate);
            v8::Local<v8::ObjectTemplate> GetObjectTemplate(void *inInfo);

            void SetFunctionTemplate(void *inInfo, v8::Local<v8::FunctionTemplate> inTemplate);
            v8::Local<v8::FunctionTemplate> GetFunctionTemplate(void *inInfo);

            void SetContextCreationHelper(JSContextCreationHelperUniquePtr inCreator);
            JSContextSharedPtr CreateContext(std::string inName);
            JSContextSharedPtr GetContextByName(std::string inName);
            void DisposeContext(std::string inName);
            void DisposeContext(JSContextSharedPtr inContext);

            void DisposeRuntime();

            JSAppSharedPtr GetApp() { return m_App; }
            std::string GetName() { return m_Name; }

            void CreateIsolate();
        protected:
            //void CreateIsolate();

            IdleTasksSupport m_IdleEnabled;
            JSAppSharedPtr m_App;
            std::string m_Name;

            // this has a custom no op deleter since we can't call delete on the pointer.
            // we do this so in our code we can use weak ptrs as protection agaisnt tasks using the isolate after
            // deletion
            V8IsolateSharedPtr m_Isolate;
            /** Owns the isolate and should disspose of it. */

            std::map<std::string, JSContextSharedPtr> m_Contextes;

            // the isolate's task queues
            std::shared_ptr<class ForegroundTaskRunner> m_TaskRunner;

            using ObjectTemplateMap = std::map<void *, v8::Eternal<v8::ObjectTemplate>>;
            using FunctionTemplateMap = std::map<void *, v8::Eternal<v8::FunctionTemplate>>;

            ObjectTemplateMap m_ObjectTemplates;
            FunctionTemplateMap m_FunctionTemplates;

            JSContextCreationHelperUniquePtr m_ContextCreation;

            JSRuntime(const JSRuntime &) = delete;
            JSRuntime &operator=(const JSRuntime &) = delete;
        };

        class JSRuntimeIsolateHelper : public PlatformIsolateHelper
        {
        public:
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(v8::Isolate *inIsolate, v8::TaskPriority priority)
            {
                JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(inIsolate);
                DCHECK_NOT_NULL(runtime);
                return runtime->GetForegroundTaskRunner();
            };
            virtual bool IdleTasksEnabled(v8::Isolate *inIsolate)
            {
                JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(inIsolate);
                DCHECK_NOT_NULL(runtime.get());
                return runtime->IdleTasksEnabled();
            };
        };
    } // namespace JSRuntime
} // namespace v8App
#endif
