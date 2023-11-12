#ifndef __V8_TYPES_H__
#define __V8_TYPES_H__

#include "v8-platform.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8IdleTaskUniquePtr = std::unique_ptr<v8::IdleTask>;
        using V8TaskUniquePtr = std::unique_ptr<v8::Task>;
        using V8JobTaskUniquePtr = std::unique_ptr<v8::JobTask>;
    }
}
#endif