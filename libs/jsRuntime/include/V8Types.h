#ifndef __V8_TYPES_H__
#define __V8_TYPES_H__

#include "v8-platform.h"
#include "v8.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8IdleTaskUniquePtr = std::unique_ptr<v8::IdleTask>;
        using V8TaskUniquePtr = std::unique_ptr<v8::Task>;
        using V8JobTaskUniquePtr = std::unique_ptr<v8::JobTask>;
        using V8IsolateSharedPtr = std::shared_ptr<v8::Isolate>;
        using V8IsolateWeakPtr = std::weak_ptr<v8::Isolate>;
        using V8TaskRunnerSharedPtr = std::shared_ptr<v8::TaskRunner>;

        using JSRuntimeWeakPtr = std::weak_ptr<class JSRuntime>;
        using JSRuntimeSharedPtr = std::shared_ptr<class JSRuntime>;
        using JSContextSharedPtr = std::shared_ptr<class JSContext>;
        using JSContextWeakPtr = std::weak_ptr<class JSContext>;
    }
}
#endif