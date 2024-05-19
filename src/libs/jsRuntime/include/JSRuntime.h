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
#include "CppBridge/CallbackHolderBase.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSContext;

        /**
         * What type of data is stored in the runtime data slots
         */
        enum class IsolateDataSlot : uint32_t
        {
            kJSRuntimeWeakPtr = 0
        };

        /**
         * Enum for controlling whether idle tasks are enabled
         */
        enum class IdleTasksSupport
        {
            kIdleTasksDisabled,
            kIdleTasksEnabled
        };

        /*
         * Used to isolate the context creation during testing
         */
        class JSContextCreationHelper
        {
        public:
            virtual ~JSContextCreationHelper() = default;
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName) = 0;
            virtual void DisposeContext(JSContextSharedPtr inContext) = 0;
        };

        /**
         * Interface for classes that need to have their handles closed in order
         * to create a snapshot
         */
        class ISnapshotHandleCloser
        {
        public:
            ~ISnapshotHandleCloser() = default;

        protected:
            virtual void CloseHandleForSnapshot() = 0;
            friend class JSRuntime;
        };
        using ISnapshotHandleCloserWeakPtr = std::weak_ptr<ISnapshotHandleCloser>;

        /**
         * Class that wrapps the v8 Isolate and provides a variety of utilitied related to it
         */
        class JSRuntime : public std::enable_shared_from_this<JSRuntime>
        {
        public:
            explicit JSRuntime(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName);
            virtual ~JSRuntime();

            JSRuntime(JSRuntime &&inRuntime); // TODO: use non default cause of the wekref

            /**
             * Creates a JSRuntime and V8 isolate and inializes the isolate for use.
             */
            static JSRuntimeSharedPtr CreateJSRuntime(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName,
                                                      const v8::StartupData *inSnapshot = nullptr, const intptr_t *inExternalReferences = nullptr, bool inForSnapshot = false);

            /**
             * Iniialies the runtime
             */
            void Initialize();

            /**
             * Gets the foreground task runner used by the isolate
             */
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner();
            /**
             * Returns whether idle tasks are enabled
             */
            virtual bool IdleTasksEnabled();

            /**
             * Gets the JSRuntime from the specified v8 isolate
             */
            static JSRuntimeSharedPtr GetJSRuntimeFromV8Isolate(v8::Isolate *inIsloate);
            /**
             * Returns a shared pointer for the v8 isolate
             */
            const V8IsolateSharedPtr GetSharedIsolate() { return m_Isolate; }
            /**
             * Gets the v8 isolate pointer
             */
            V8Isolate *GetIsolate() { return m_Isolate.get(); }

            /**
             * Runs the isolates tasks
             */
            void ProcessTasks();
            /**
             * Runs the idle tasks for the isolate4
             */
            void ProcessIdleTasks(double inTimeLeft);

            /**
             * Stores the Object tamplate for the isolate
             */
            void SetObjectTemplate(void *inInfo, v8::Local<v8::ObjectTemplate> inTemplate);
            /**
             * Gets an object template for the isolate
             */
            v8::Local<v8::ObjectTemplate> GetObjectTemplate(void *inInfo);

            /**
             * Stores a function template for the isolate
             */
            void SetFunctionTemplate(intptr_t inFuncAddress, CppBridge::CallbackHolderBase* inHolder);
            /**
             * Gets a function template for the isolate
             */
           CppBridge::CallbackHolderBase* GetFunctionTemplate(intptr_t inFuncAddress);

            void RegisterTemplatesOnGlobal(v8::Local<v8::ObjectTemplate> &inObject);

            /**
             * Sets the context creation helper
             */
            void SetContextCreationHelper(JSContextCreationHelperSharedPtr inCreator);
            /**
             * Gets the context creator for this runtime
             */
            JSContextCreationHelperSharedPtr GetContextCreationHelper();

            /**
             * Creates a JSContext with the specified name
             */
            JSContextSharedPtr CreateContext(std::string inName);
            /**
             * Gets the JSContext with the specified name
             */
            JSContextSharedPtr GetContextByName(std::string inName);
            /**
             * Disposes of the JSContext with the specified name
             */
            void DisposeContext(std::string inName);
            /**
             * Dispose of the JSContext pased in the shared pointer
             */
            void DisposeContext(JSContextSharedPtr inContext);

            /**
             * Disposes of reousrces for the runtime
             */
            void DisposeRuntime();

            /**
             * Gtes hte JSApp associated with the Runtime
             */
            JSAppSharedPtr GetApp() { return m_App; }
            /**
             * Gets the name of the runtime
             */
            std::string GetName() { return m_Name; }

            /**
             * Gets the snapshot creator for the app.
             */
            V8SnapshotCreatorSharedPtr GetSnapshotCreator();

            /**
             * Closes all open handles for snapshoting
             */
            void CloseOpenHandlesForSnapshot();
            /**
             * Register a callback to close a handle for the isolate
             */
            void RegisterSnapshotHandleCloser(ISnapshotHandleCloserWeakPtr inCloser);
            void UnregisterSnapshotHandlerCloser(ISnapshotHandleCloserWeakPtr inCloser);

            v8::Local<v8::ObjectTemplate> GetGlobalTemplate() { return m_GlobalTemplate.Get(m_Isolate.get()); }
            void CreateGlobalTemplate(bool inRegister);

        protected:
            /**
             * Creates the v8 isolate and if for a snapshot the v8 snapshot creator as well
             */
            void CreateIsolate(const v8::StartupData *inSnapshot, const intptr_t *inExternalReferences, bool inForSnapshot);

            IdleTasksSupport m_IdleEnabled;
            JSAppSharedPtr m_App;
            std::string m_Name;

            /**
             * Cllbacks to close out the handles when the isolate is snapshotting
             */
            std::vector<ISnapshotHandleCloserWeakPtr> m_HandleClosers;

            /**
             * Has a custom deleter to call dispose on the isolate
             */
            V8IsolateSharedPtr m_Isolate;

            /**
             * The contexts associated with the runtime
             */
            std::map<std::string, JSContextSharedPtr> m_Contextes;

            /**
             * The task runner for the isolate
             */
            std::shared_ptr<class ForegroundTaskRunner> m_TaskRunner;

            using ObjectTemplateMap = std::map<void *, v8::Eternal<v8::ObjectTemplate>>;
            using FunctionTemplateMap = std::map<intptr_t, CppBridge::CallbackHolderBase*>;

            /**
             * The object templates for the isolate
             */
            ObjectTemplateMap m_ObjectTemplates;
            /**
             * The function templates for the isolate
             */
            FunctionTemplateMap m_FunctionTemplates;

            V8PersistentObjTpl m_GlobalTemplate;
            /**
             * The Context Creator
             */
            JSContextCreationHelperSharedPtr m_ContextCreation;

            /**
             * The v8 SnapshotCreator
             */
            V8SnapshotCreatorSharedPtr m_Creator;

            JSRuntime(const JSRuntime &) = delete;
            JSRuntime &operator=(const JSRuntime &) = delete;
        };

        /**
         * The helper class that implments the Platform Isolate helper for JSRuntimes
         */
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
