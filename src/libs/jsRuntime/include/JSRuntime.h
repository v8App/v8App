// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_RUNTIME_H_
#define _JS_RUNTIME_H_

#include <memory>
#include <map>
#include <filesystem>

#include "Utils/Format.h"

#include "ISnapshotHandleCloser.h"
// 4#include "JSApp.h"
#include "V8Types.h"
#include "V8AppPlatform.h"
// #include "JSSnapshotCreator.h"
// #include "CppBridge/CallbackHolderBase.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSContext;

        /**
         * Enum for controlling whether idle tasks are enabled
         */
        enum class IdleTasksSupport
        {
            kIdleTasksDisabled,
            kIdleTasksEnabled
        };

        /**
         * Internal field for the Native Object.
         * Declared here since we need to provide the indexes
         * to the CppHeap
         */
        enum class V8CppObjDataIntField : int
        {
            CppHeapID,
            ObjInfo,
            ObjInstance,
            MaxInternalFields
        };

        /**
         * Class that wrapps the v8 Isolate and provides a variety of utilitied related to it
         */
        class JSRuntime : public std::enable_shared_from_this<JSRuntime>
        {
        public:
            /**
             * What type of data is stored in the runtime data slots
             */
            enum class DataSlot : uint32_t
            {
                kJSRuntimeWeakPtr = 0
            };

            /**
             * The max number of contexes we support in a snapshot
             */
            static inline const int kMaxContextNamespaces = 1024;

            explicit JSRuntime(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName);
            virtual ~JSRuntime();

            JSRuntime(JSRuntime &&inRuntime) = default; // TODO: use non default cause of the wekref

            /**
             * Creates a JSRuntime and V8 isolate and inializes the isolate for use.
             */
            static JSRuntimeSharedPtr CreateJSRuntime(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName,
                                                      JSContextCreationHelperSharedPtr inContextCreator, bool inForSnapshot = false);

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
            static JSRuntimeSharedPtr GetJSRuntimeFromV8Isolate(V8Isolate *inIsloate);
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
             * Sets the context creation helper
             */
            void SetContextCreationHelper(JSContextCreationHelperSharedPtr inCreator);
            /**
             * Gets the context creator for this runtime
             */
            JSContextCreationHelperSharedPtr GetContextCreationHelper();

            /**
             * Creates a JSContext with the specified namespace.
             * If no namespace is provided then returns the default v8 context
             */
            JSContextSharedPtr CreateContext(std::string inName, std::filesystem::path inEntryPoint, std::string inNamespace = "",
                                             std::filesystem::path inSnapEntryPoint = "", bool inSupportsSnapshot = true,
                                             SnapshotMethod inSnapMethod = SnapshotMethod::kNamespaceOnly);
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
             * Gets the name of the snapshot index for a context
             */
            std::string GetNamespaceForSnapIndex(size_t inSnapIndex);
            /**
             * Gets the snapshot index for the given name
             */
            size_t GetSnapIndexForNamespace(std::string inNamespace);
            /**
             * Adds a snapshot index with the given name
             */
            bool AddSnapIndexNamespace(size_t inSnapIndex, std::string inNamespace);

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
            /**
             * Unregister a close handler
             */
            void UnregisterSnapshotHandlerCloser(ISnapshotHandleCloserWeakPtr inCloser);

            /**
             * Gets the cppgc heap for the isolate
             */
            V8CppHeap *GetCppHeap();
            /**
             * Gets hte id to use for the cppgc heap to mark whether they should be scanned
             */
            inline uint16_t *GetCppHeapID() { return &m_CppHeapID; }

        protected:
            /**
             * Creates the v8 isolate and if for a snapshot the v8 snapshot creator as well
             */
            bool CreateIsolate(bool inForSnapshot);

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

            /**
             * The Context Creator
             */
            JSContextCreationHelperSharedPtr m_ContextCreation;

            using ObjectTemplateMap = std::map<void *, v8::Global<v8::ObjectTemplate>>;

            /**
             * The object templates for the isolate
             */
            ObjectTemplateMap m_ObjectTemplates;

            /**
             * The v8 SnapshotCreator
             */
            V8SnapshotCreatorSharedPtr m_Creator;

            /**
             * Heap ID for the CppHeap
             */
            uint16_t m_CppHeapID = 1;

            /**
             * Map of the snapshot indexs to names to create contextes from the snapshot
             */
            std::map<size_t, std::string> m_SnapshotContextNames;

            JSRuntime(const JSRuntime &) = delete;
            JSRuntime &operator=(const JSRuntime &) = delete;
        };

        /**
         * The helper class that implments the Platform Isolate helper for JSRuntimes
         */
        class JSRuntimeIsolateHelper : public PlatformIsolateHelper
        {
        public:
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(V8Isolate *inIsolate, V8TaskPriority priority) override;
            virtual bool IdleTasksEnabled(V8Isolate *inIsolate) override;
        };
    } // namespace JSRuntime
} // namespace v8App
#endif
