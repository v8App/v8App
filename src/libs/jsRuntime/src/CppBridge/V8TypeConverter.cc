// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "CppBridge/V8TypeConverter.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            template <typename T, typename U>
            bool FromMaybe(v8::Maybe<T> inMaybe, U *outValue)
            {
                if (inMaybe.IsNothing())
                {
                    return false;
                }
                *outValue = static_cast<U>(inMaybe.FromJust());
                return true;
            }

            V8LValue V8TypeConverter<bool>::To(V8Isolate *inIsolate, bool inValue)
            {
                return V8Boolean::New(inIsolate, inValue).As<V8Value>();
            }

            bool V8TypeConverter<bool>::From(V8Isolate *inIsolate, V8LValue inValue, bool *outValue)
            {
                *outValue = inValue->BooleanValue(inIsolate);
                return true;
            }

            V8LValue V8TypeConverter<int32_t>::To(V8Isolate *inIsolate, int32_t inValue)
            {
                return V8Integer::New(inIsolate, inValue).As<V8Value>();
            }

            bool V8TypeConverter<int32_t>::From(V8Isolate *inIsolate, V8LValue inValue, int32_t *outValue)
            {
                if (inValue->IsInt32())
                {
                    *outValue = inValue.As<v8::Int32>()->Value();
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<uint32_t>::To(V8Isolate *inIsolate, uint32_t inValue)
            {
                return V8Integer::NewFromUnsigned(inIsolate, inValue).As<V8Value>();
            }

            bool V8TypeConverter<uint32_t>::From(V8Isolate *inIsolate, V8LValue inValue, uint32_t *outValue)
            {
                if (inValue->IsUint32())
                {
                    *outValue = inValue.As<v8::Uint32>()->Value();
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<int64_t>::To(V8Isolate *inIsolate, int64_t inValue)
            {
                return V8Number::New(inIsolate, static_cast<double>(inValue)).As<V8Value>();
            }

            bool V8TypeConverter<int64_t>::From(V8Isolate *inIsolate, V8LValue inValue, int64_t *outValue)
            {
                if (inValue->IsNumber())
                {
                    return FromMaybe(inValue->IntegerValue(inIsolate->GetCurrentContext()), outValue);
                }
                return false;
            }

            V8LValue V8TypeConverter<uint64_t>::To(V8Isolate *inIsolate, uint64_t inValue)
            {
                return V8Number::New(inIsolate, static_cast<double>(inValue)).As<V8Value>();
            }

            bool V8TypeConverter<uint64_t>::From(V8Isolate *inIsolate, V8LValue inValue, uint64_t *outValue)
            {
                if (inValue->IsNumber())
                {
                    return FromMaybe(inValue->IntegerValue(inIsolate->GetCurrentContext()), outValue);
                }
                return false;
            }

            V8LValue V8TypeConverter<float>::To(V8Isolate *inIsolate, float inValue)
            {
                return V8Number::New(inIsolate, inValue).As<V8Value>();
            }

            bool V8TypeConverter<float>::From(V8Isolate *inIsolate, V8LValue inValue, float *outValue)
            {
                if (inValue->IsNumber())
                {
                    *outValue = static_cast<float>(inValue.As<V8Number>()->Value());
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<double>::To(V8Isolate *inIsolate, double inValue)
            {
                return V8Number::New(inIsolate, inValue).As<V8Value>();
            }

            bool V8TypeConverter<double>::From(V8Isolate *inIsolate, V8LValue inValue, double *outValue)
            {
                if (inValue->IsNumber())
                {
                    *outValue = inValue.As<V8Number>()->Value();
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<std::string>::To(V8Isolate *inIsolate, const std::string &inValue)
            {
                return V8String::NewFromUtf8(inIsolate, inValue.c_str(), v8::NewStringType::kNormal, inValue.length()).ToLocalChecked();
            }

            bool V8TypeConverter<std::string>::From(V8Isolate *inIsolate, V8LValue inValue, std::string *outValue)
            {
                if (inValue->IsString())
                {
                    V8LString v8Str = V8LString::Cast(inValue);
                    int length = v8Str->Utf8Length(inIsolate);
                    outValue->resize(length);
                    v8Str->WriteUtf8(inIsolate, &(*outValue)[0], length, nullptr, V8String::NO_NULL_TERMINATION);
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<std::u16string>::To(V8Isolate *inIsolate, const std::u16string &inValue, v8::NewStringType inStringType)
            {
                return V8String::NewFromTwoByte(inIsolate, reinterpret_cast<const uint16_t *>(inValue.c_str()), inStringType, inValue.length()).ToLocalChecked();
            }

            bool V8TypeConverter<std::u16string>::From(V8Isolate *inIsolate, V8LValue inValue, std::u16string *outValue)
            {
                if (inValue->IsString())
                {
                    V8LString v8Str = V8LString::Cast(inValue);
                    outValue->resize(v8Str->Length());
                    v8Str->Write(inIsolate, reinterpret_cast<uint16_t *>(&(*outValue)[0]), 0, v8Str->Length(), V8String::NO_NULL_TERMINATION);
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<V8LFunction>::To(V8Isolate *inIsolate, V8LFunction inValue)
            {
                return inValue.As<V8Value>();
            }

            bool V8TypeConverter<V8LFunction>::From(V8Isolate *inIsolate, V8LValue inValue, V8LFunction *outValue)
            {
                if (inValue->IsFunction())
                {
                    *outValue = V8LFunction::Cast(inValue);
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<V8LObject>::To(V8Isolate *inIsolate, V8LObject inValue)
            {
                return inValue.As<V8Value>();
            }

            bool V8TypeConverter<V8LObject>::From(V8Isolate *inIsolate, V8LValue inValue, V8LObject *outValue)
            {
                if (inValue->IsObject())
                {
                    *outValue = V8LObject::Cast(inValue);
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<V8LPromise>::To(V8Isolate *inIsolate, V8LPromise inValue)
            {
                return inValue.As<V8Value>();
            }

            bool V8TypeConverter<V8LPromise>::From(V8Isolate *inIsolate, V8LValue inValue, V8LPromise *outValue)
            {
                if (inValue->IsPromise())
                {
                    *outValue = V8LPromise::Cast(inValue);
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<V8LArrayBuffer>::To(V8Isolate *inIsolate, V8LArrayBuffer inValue)
            {
                return inValue.As<V8Value>();
            }

            bool V8TypeConverter<V8LArrayBuffer>::From(V8Isolate *inIsolate, V8LValue inValue, V8LArrayBuffer *outValue)
            {
                if (inValue->IsArrayBuffer())
                {
                    *outValue = V8LArrayBuffer::Cast(inValue);
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<V8LExternal>::To(V8Isolate *inIsolate, V8LExternal inValue)
            {
                return inValue.As<V8Value>();
            }

            bool V8TypeConverter<V8LExternal>::From(V8Isolate *inIsolate, V8LValue inValue, V8LExternal *outValue)
            {
                if (inValue->IsExternal())
                {
                    *outValue = V8LExternal::Cast(inValue);
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<V8LValue>::To(V8Isolate *inIsolate, V8LValue inValue)
            {
                return inValue;
            }

            bool V8TypeConverter<V8LValue>::From(V8Isolate *inIsolate, V8LValue inValue, V8LValue *outValue)
            {
                *outValue = inValue;
                return true;
            }

            V8LValue V8TypeConverter<V8LNumber>::To(V8Isolate *inIsolate, V8LNumber inValue)
            {
                return inValue.As<V8Value>();
            }

            bool V8TypeConverter<V8LNumber>::From(V8Isolate *inIsolate, V8LValue inValue, V8LNumber *outValue)
            {
                if (inValue->IsNumber())
                {
                    *outValue = V8LNumber::Cast(inValue);
                    return true;
                }
                return false;
            }

            V8LValue V8TypeConverter<V8LBigInt>::To(V8Isolate *inIsolate, V8LBigInt inValue)
            {
                return inValue.As<V8Value>();
            }

            bool V8TypeConverter<V8LBigInt>::From(V8Isolate *inIsolate, V8LValue inValue, V8LBigInt *outValue)
            {
                if (inValue->IsBigInt())
                {
                    *outValue = V8LBigInt::Cast(inValue);
                    return true;
                }
                return false;
            }
        }
    }
}