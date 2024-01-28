#ifndef __V8_TYPES_H__
#define __V8_TYPES_H__

#include "v8-platform.h"
#include "v8.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8Isolate = v8::Isolate;

        using V8LocalContext = v8::Local<v8::Context>;
        using V8GlobalContext = v8::Global<v8::Context>;
        using V8MaybeLocalContext = v8::MaybeLocal<v8::Context>;

        using V8LocalModule = v8::Local<v8::Module>;
        using V8GlobalModule = v8::Global<v8::Module>;
        using V8MaybeLocalModule = v8::MaybeLocal<v8::Module>;

        using V8LocalString = v8::Local<v8::String>;
        using V8GlobalString = v8::Global<v8::String>;

        using V8LocalFixedArray = v8::Local<v8::FixedArray>;
        using V8GlobalFixedArray = v8::Global<v8::FixedArray>;

        using V8LocalData = v8::Local<v8::Data>;

        using V8LocalValue = v8::Local<v8::Value>;
        using V8MaybeLocalValue = v8::MaybeLocal<v8::Value>;
        using V8GlobalValue = v8::Global<v8::Value>;

        using V8LocalObject = v8::Local<v8::Object>;
        using V8GlobalObject = v8::Global<v8::Object>;

        using V8FuncCallInfoValue = v8::FunctionCallbackInfo<v8::Value>;

        using V8LocalPromise = v8::Local<v8::Promise>;
        using V8MaybeLocalPromise = v8::MaybeLocal<v8::Promise>;
        using V8LocalPromiseResolver = v8::Local<v8::Promise::Resolver>;
        using V8GlobalPromiseResolver = v8::Global<v8::Promise::Resolver>;

        using V8ScriptCachedData = v8::ScriptCompiler::CachedData;
        using V8ScriptCachedDataUniquePtr = std::unique_ptr<v8::ScriptCompiler::CachedData>;

        using V8ScriptSource = v8::ScriptCompiler::Source;
        using V8ScriptSourceUniquePtr = std::unique_ptr<v8::ScriptCompiler::Source>;
        
        using V8LocalUnboundModuleScript = v8::Local<v8::UnboundModuleScript>;

        using V8LocalModuleRequst = v8::Local<v8::ModuleRequest>;
        
        using V8IdleTaskUniquePtr = std::unique_ptr<v8::IdleTask>;
        using V8TaskUniquePtr = std::unique_ptr<v8::Task>;
        using V8JobTaskUniquePtr = std::unique_ptr<v8::JobTask>;
        using V8IsolateSharedPtr = std::shared_ptr<v8::Isolate>;
        using V8IsolateWeakPtr = std::weak_ptr<v8::Isolate>;
        using V8TaskRunnerSharedPtr = std::shared_ptr<v8::TaskRunner>;

        using JSAppSharedPtr = std::shared_ptr<class JSApp>;
        using JSAppWeakPtr = std::weak_ptr<class JSApp>;

        using CodeCacheSharedPtr = std::shared_ptr<class CodeCache>;

        using JSRuntimeWeakPtr = std::weak_ptr<class JSRuntime>;
        using JSRuntimeSharedPtr = std::shared_ptr<class JSRuntime>;

        using JSContextSharedPtr = std::shared_ptr<class JSContext>;
        using JSContextWeakPtr = std::weak_ptr<class JSContext>;

        using JSContextModulesWeakPtr = std::weak_ptr<class JSContextModules>;
        using JSContextModulesSharedPtr = std::shared_ptr<JSContextModules>;
    }
}
#endif