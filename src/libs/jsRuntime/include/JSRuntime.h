// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_RUNTIME_H_
#define _JS_RUNTIME_H_

#include <memory>
#include <map>
#include <filesystem>

// #include "Utils/Format.h"
#include "Containers/NamedIndexes.h"

#include "ForegroundTaskRunner.h"
#include "ISnapshotHandleCloser.h"
#include "IJSPlatformRuntimeProvider.h"
#include "V8Types.h"
#include "ISnapshotObject.h"
#include "JSRuntimeSnapData.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSContext;

        /**
         * Class that wrapps the v8 Isolate and provides a variety of utilitied related to it
         */
        class JSRuntime : public std::enable_shared_from_this<JSRuntime>, public ISnapshotObject
        {
        public:
            inline static const char *kDefaultV8ContextName = "v8-default";
            inline static const size_t kDefaultV8ContextIndex = 0;

            /**
             * What type of data is stored in the runtime data slots
             */
            enum class DataSlot : uint32_t
            {
                kJSRuntimeWeakPtr = 0
            };

            JSRuntime();
            virtual ~JSRuntime();

            JSRuntime(JSRuntime &&inRuntime);

            /**
             * Iniialies the runtime
             */
            bool Initialize(JSAppSharedPtr inApp, std::string inName, size_t inRuntimeIndex = 0, JSRuntimeSnapshotAttributes inSnapAttribute = JSRuntimeSnapshotAttributes::NotSnapshottable,
                            bool isSnapshottable = false, IdleTaskSupport inEnableIdle = IdleTaskSupport::kEnabled);

            /**
             * Gets the foreground task runner used by the isolate
             */
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner() { return m_TaskRunner; }
            /**
             * Returns whether idle tasks are enabled
             */
            virtual bool IdleTasksEnabled() { return m_IdleEnabled == IdleTaskSupport::kEnabled; }

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
             * Dispose of the JSContext pased in the shared pointer
             */
            void DisposeContext(JSContextSharedPtr inContext);
            /**
             * Disposes of the JSContext with the specified name
             */
            void DisposeContext(std::string inName);

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
            V8SnapshotCreatorSharedPtr GetSnapshotCreator() { return m_Creator; }

            /**
             * Returns if this runtime is a initialized as snapshot one
             */
            bool IsSnapshotRuntime() { return m_IsSnapshotter; }

            /**
             * Whether the runtime can be snapshotted. The runtime may not be
             * initialized as a snapshot isolate but can be cloned into one
             */
            bool CanBeSnapshotted() { return m_Snapshottable != JSRuntimeSnapshotAttributes::NotSnapshottable; }

            /**
             * Closes all open handles for snapshoting
             */
            void CloseOpenHandlesForSnapshot();
            /**
             * Register a callback to close a handle for the isolate
             */
            void RegisterSnapshotHandleCloser(ISnapshotHandleCloser *inCloser);
            /**
             * Unregister a close handler, use a ptr since we could be in a destructor
             * and shared_ptr is no longer valid
             */
            void UnregisterSnapshotHandlerCloser(ISnapshotHandleCloser *inCloser);

            /**
             * Gets the cppgc heap for the isolate
             */
            V8CppHeap *GetCppHeap();
            /**
             * Gets hte id to use for the cppgc heap to mark whether they should be scanned
             */
            inline uint16_t *GetCppHeapID() { return &m_CppHeapID; }

            /**
             * Gets the context provider returning the app's or if one was passed when the runtime was inited
             */
            IJSContextProviderSharedPtr GetContextProvider();

            /**
             * Sets a custom context provider on the runtime different from what the app has
             */
            void SetContextProvider(IJSContextProviderSharedPtr inProvider);

            bool IsInitialzed() { return m_Initialized; }

            size_t GetSnapshotIndex() { return m_SnapshotIndex; }

            virtual bool MakeSnapshot(Serialization::WriteBuffer &inBuffer, void *inData = nullptr);
            virtual JSRuntimeSnapDataSharedPtr LoadSnapshotData(Serialization::ReadBuffer &inBuffer);
            virtual bool RestoreSnapshot(JSRuntimeSnapDataSharedPtr inSnapData);

        protected:
            /**
             * Uses the name context's name, namespace and snap method to reolve the context's index name in the snapshot
             * Naemspace only will only use the name space to look up the index.
             * NamespaceAndEntryPoint will use the name and namespace to resolve the name
             */
            std::string ResolveContextName(std::string inName, std::string inNamespace, SnapshotMethod inMethod);

            /**
             * Creates the v8 isolate and if for a snapshot the v8 snapshot creator as well
             */
            bool CreateIsolate();

            /**
             * Clones the passin runtime for snapshotting. This will onyl copy contextex
             * that support snapshotting, runs the snap shot entry point if specified or the entry point script
             * if one is passed.
             */
            JSRuntimeSharedPtr CloneRuntimeForSnapshotting(JSAppSharedPtr inApp);

            /**
             * Go through the contextes and adds them to the snapshot creator
             * and records their indexes for serialization
             */
            bool AddContextesToSnapshot();

            /** Wheter idle tasks are enabled */
            IdleTaskSupport m_IdleEnabled;
            /** Teh JSApp */
            JSAppSharedPtr m_App;
            /** the name of the runtime */
            std::string m_Name;
            /** Is the runtime already inited */
            bool m_Initialized;

            /**
             * Cllbacks to close out the handles when the isolate is snapshotting
             */
            std::vector<ISnapshotHandleCloser *> m_HandleClosers;

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
            std::shared_ptr<ForegroundTaskRunner> m_TaskRunner;

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
             * Is this a snapshot runtime
             */
            bool m_IsSnapshotter;

            /**
             * can be snapshotted
             */
            JSRuntimeSnapshotAttributes m_Snapshottable{JSRuntimeSnapshotAttributes::NotSnapshottable};

            /**
             * Heap ID for the CppHeap
             */
            uint16_t m_CppHeapID = 1;

            /**
             * If a custom context provider was passed in during init
             */
            IJSContextProviderSharedPtr m_CustomContextProvider;

            /**
             * Which index the the snapshot data was loaded from
             */
            size_t m_SnapshotIndex;

            Containers::NamedIndexes m_ContextNamespaces;

            JSRuntime(const JSRuntime &) = delete;
            JSRuntime &operator=(const JSRuntime &) = delete;

            friend class JSApp;
            friend class JSSnapshotCreator;
        };

        /**
         * The helper class that implments the Platform Isolate helper for JSRuntimes
         */
        class JSRuntimeIsolateHelper : public IJSPlatformRuntimeProvider
        {
        public:
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(V8Isolate *inIsolate, V8TaskPriority priority) override;
            virtual bool IdleTasksEnabled(V8Isolate *inIsolate) override;
        };

    } // namespace JSRuntime

    template <>
    struct Serialization::TypeSerializer<v8App::JSRuntime::IdleTaskSupport>
    {
        static bool Serialize(BaseBuffer &inBuffer, v8App::JSRuntime::IdleTaskSupport &inValue);
    };
} // namespace v8App
#endif
