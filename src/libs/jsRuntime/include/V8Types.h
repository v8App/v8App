#ifndef __V8_TYPES_H__
#define __V8_TYPES_H__

#include "v8/v8-platform.h"
#include "v8/libplatform/libplatform.h"
#include "v8/v8-snapshot.h"
#include "v8/v8-cppgc.h"
#include "v8/v8.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * v8 Types
         * These are shortened and don't require you to type the ::
         *
         * Short Hands used
         * V8L = v8::Local
         * V8G = v8::Global
         * V8MBL = v8::MaybeLocal
         *
         */
        using V8Isolate = v8::Isolate;
        using V8IsolateSharedPtr = std::shared_ptr<v8::Isolate>;
        using V8IsolateScope = v8::Isolate::Scope;

        using V8Locker = v8::Locker;

        using V8Context = v8::Context;
        using V8LContext = v8::Local<v8::Context>;
        using V8GContext = v8::Global<v8::Context>;
        using V8MBLContext = v8::MaybeLocal<v8::Context>;
        using V8ContextScope = v8::Context::Scope;

        using V8Module = v8::Module;
        using V8LModule = v8::Local<v8::Module>;
        using V8GModule = v8::Global<v8::Module>;
        using V8MBLModule = v8::MaybeLocal<v8::Module>;

        using V8ModRequest = v8::ModuleRequest;
        using V8LModRequest = v8::Local<v8::ModuleRequest>;

        using V8LData = v8::Local<v8::Data>;

        using V8Exception = v8::Exception;

        using V8Value = v8::Value;
        using V8LValue = v8::Local<v8::Value>;
        using V8MBLValue = v8::MaybeLocal<v8::Value>;
        using V8GValue = v8::Global<v8::Value>;

        using V8String = v8::String;
        using V8LString = v8::Local<v8::String>;
        using V8MBLString = v8::MaybeLocal<v8::String>;
        using V8GlobalString = v8::Global<v8::String>;

        using V8FixedArray = v8::FixedArray;
        using V8LFixedArray = v8::Local<v8::FixedArray>;
        using V8GlobalFixedArray = v8::Global<v8::FixedArray>;

        using V8Object = v8::Object;
        using V8LObject = v8::Local<v8::Object>;
        using V8MBLObject = v8::MaybeLocal<v8::Object>;
        using V8GObject = v8::Global<v8::Object>;

        using V8External = v8::External;
        using V8LExternal = v8::Local<v8::External>;
        using V8GExternal = v8::Global<v8::External>;

        using V8BigInt = v8::BigInt;
        using V8LBigInt = v8::Local<v8::BigInt>;

        using V8ArrayBuffer = v8::ArrayBuffer;
        using V8LArrayBuffer = v8::Local<v8::ArrayBuffer>;

        using V8Number = v8::Number;
        using V8LNumber = v8::Local<v8::Number>;

        using V8Boolean = v8::Boolean;
        using V8LBoolean = v8::Local<v8::Boolean>;

        using V8Integer = v8::Integer;
        using V8LInteger = v8::Local<v8::Integer>;

        using V8Array = v8::Array;
        using V8LArray = v8::Local<v8::Array>;

        using V8Message = v8::Message;
        using V8LMessage = v8::Local<v8::Message>;

        using V8StackTrace = v8::StackTrace;
        using V8LStackTrace = v8::Local<v8::StackTrace>;

        using V8StackFrame = v8::StackFrame;
        using V8LStackFrame = v8::Local<v8::StackFrame>;

        using V8FuncCallInfoValue = v8::FunctionCallbackInfo<v8::Value>;
        using V8PropCallInfoValeu = v8::PropertyCallbackInfo<v8::Value>;

        using V8FuncTpl = v8::FunctionTemplate;
        using V8LFuncTpl = v8::Local<v8::FunctionTemplate>;

        using V8ObjTpl = v8::ObjectTemplate;
        using V8LObjTpl = v8::Local<v8::ObjectTemplate>;
        using V8GObjTpl = v8::Global<v8::ObjectTemplate>;

        using V8Function = v8::Function;
        using V8LFunction = v8::Local<v8::Function>;

        using V8Promise = v8::Promise;
        using V8LPromise = v8::Local<v8::Promise>;
        using V8MBLPromise = v8::MaybeLocal<v8::Promise>;

        using V8PromiseResolver = v8::Promise::Resolver;
        using V8LPromiseResolver = v8::Local<v8::Promise::Resolver>;
        using V8MBLPromiseResolver = v8::MaybeLocal<v8::Promise::Resolver>;
        using V8GPromiseResolver = v8::Global<v8::Promise::Resolver>;

        using V8ScriptCompiler = v8::ScriptCompiler;

        using V8ScriptCachedData = v8::ScriptCompiler::CachedData;
        using V8ScriptCachedDataUniquePtr = std::unique_ptr<v8::ScriptCompiler::CachedData>;

        using V8ScriptSource = v8::ScriptCompiler::Source;
        using V8ScriptSourceUniquePtr = std::unique_ptr<v8::ScriptCompiler::Source>;

        using V8LScript = v8::Local<v8::Script>;
        using V8MLScript = v8::MaybeLocal<v8::Script>;

        using V8ScriptOrigin = v8::ScriptOrigin;

        using V8SourceLocation = v8::SourceLocation;

        using V8LUnboundModScript = v8::Local<v8::UnboundModuleScript>;
        using V8GUnboundModScript = v8::Global<v8::UnboundModuleScript>;

        using V8TryCatch = v8::TryCatch;

        using V8SnapCreator = v8::SnapshotCreator;
        using V8SnapshotCreatorSharedPtr = std::shared_ptr<v8::SnapshotCreator>;

        using V8StartupData = v8::StartupData;

        using V8HandleScope = v8::HandleScope;

        using V8CppHeap = v8::CppHeap;
        using V8CppHeapUniquePtr = std::unique_ptr<V8CppHeap>;

        using V8Platform = v8::Platform;

        using V8TaskPriority = v8::TaskPriority;

        using V8IdleTask = v8::IdleTask;
        using V8IdleTaskUniquePtr = std::unique_ptr<v8::IdleTask>;

        using V8Task = v8::Task;
        using V8TaskUniquePtr = std::unique_ptr<v8::Task>;

        using V8JobTask = v8::JobTask;
        using V8JobTaskUniquePtr = std::unique_ptr<v8::JobTask>;

        using V8JobHandleUniquePtr = std::unique_ptr<v8::JobHandle>;

        using V8TaskRunner = v8::TaskRunner;
        using V8TaskRunnerSharedPtr = std::shared_ptr<v8::TaskRunner>;

        using V8TracingController = v8::TracingController;
        using V8TracingControllerUniquePtr = std::unique_ptr<v8::TracingController>;

        using V8PageAllocator = v8::PageAllocator;
        using V8PageAllocatorUniquePtr = std::unique_ptr<v8::PageAllocator>;

        using V8ThreadIsolatedAllocator = v8::ThreadIsolatedAllocator;
        using V8ThreadIsolatedAllocatorUniquePtr = std::unique_ptr<v8::ThreadIsolatedAllocator>;

        using V8HighAllocationThroughputObserver = v8::HighAllocationThroughputObserver;
        using V8HighAllocationThroughputObserverUniquePtr = std::unique_ptr<v8::HighAllocationThroughputObserver>;

        using V8ZoneBackingAllocator = v8::ZoneBackingAllocator;
        using V8ZoneBackingAllocatorUniquePtr = std::unique_ptr<v8::ZoneBackingAllocator>;

        using V8ScopedBlockingCall = v8::ScopedBlockingCall;
        using V8ScopedBlockingCallUniquePtr = std::unique_ptr<v8::ScopedBlockingCall>;

        using V8BlockingType = v8::BlockingType;

        using IdleTaskSupport = v8::platform::IdleTaskSupport;

        /**
         * v8App Types
         */
        using PlatformRuntimeProviderUniquePtr = std::unique_ptr<class IJSPlatformRuntimeProvider>;

        using JSAppSharedPtr = std::shared_ptr<class JSApp>;

        using CodeCacheSharedPtr = std::shared_ptr<class CodeCache>;

        using JSRuntimeWeakPtr = std::weak_ptr<class JSRuntime>;
        using JSRuntimeSharedPtr = std::shared_ptr<class JSRuntime>;

        using IJSSnapshotProviderSharedPtr = std::shared_ptr<class IJSSnapshotProvider>;
        using IJSSnapshotCreatorSharedPtr = std::shared_ptr<class IJSSnapshotCreator>;
        using IJSRuntimeProviderSharedPtr = std::shared_ptr<class IJSRuntimeProvider>;
        using IJSContextProviderSharedPtr = std::shared_ptr<class IJSContextProvider>;

        using V8SnapshotProviderSharedPtr = std::shared_ptr<class V8SnapshotProvider>;

        using JSContextSharedPtr = std::shared_ptr<class JSContext>;
        using JSContextWeakPtr = std::weak_ptr<class JSContext>;

        using JSContextModulesWeakPtr = std::weak_ptr<class JSContextModules>;
        using JSContextModulesSharedPtr = std::shared_ptr<JSContextModules>;

        using JSSnapshotCreatorUniquePtr = std::unique_ptr<class JSSnapshotCreator>;

        using ISnapshotHandleCloserSharedPtr = std::shared_ptr<class ISnapshotHandleCloser>;
        using ISnapshotHandleCloserWeakPtr = std::weak_ptr<class ISnapshotHandleCloser>;

        /**
         * The method to use when snapshotting the context.
         * kNamespaceOnly will only setup the global object if needed
         * kNamespaceAndEntrypoint with setup up the global object and run the entry point script
         */
        enum class SnapshotMethod : int
        {
            kNamespaceOnly,
            kNamespaceAndEntrypoint
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
         * Struct that holds the provider classes to manage runtimes, contextes and snapshots
         */
        struct AppProviders
        {
            AppProviders() {}
            AppProviders(IJSSnapshotProviderSharedPtr inSnapshotProvider, IJSRuntimeProviderSharedPtr inRuntimeProvider,
                         IJSContextProviderSharedPtr inContextProvidr, IJSSnapshotCreatorSharedPtr inSnapshotCreator = nullptr)
                : m_SnapshotProvider(inSnapshotProvider),
                  m_RuntimeProvider(inRuntimeProvider),
                  m_ContextProvider(inContextProvidr),
                  m_SnapshotCreator(inSnapshotCreator) {}

            IJSSnapshotProviderSharedPtr m_SnapshotProvider;
            IJSRuntimeProviderSharedPtr m_RuntimeProvider;
            IJSContextProviderSharedPtr m_ContextProvider;
            IJSSnapshotCreatorSharedPtr m_SnapshotCreator;
        };

    }
}
#endif //__V8_TYPES_H__