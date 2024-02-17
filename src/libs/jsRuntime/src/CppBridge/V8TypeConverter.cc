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

            v8::Local<v8::Value> V8TypeConverter<bool>::To(v8::Isolate *inIsolate, bool inValue)
            {
                return v8::Boolean::New(inIsolate, inValue).As<v8::Value>();
            }

            bool V8TypeConverter<bool>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, bool *outValue)
            {
                *outValue = inValue->BooleanValue(inIsolate);
                return true;
            }

            v8::Local<v8::Value> V8TypeConverter<int32_t>::To(v8::Isolate *inIsolate, int32_t inValue)
            {
                return v8::Integer::New(inIsolate, inValue).As<v8::Value>();
            }

            bool V8TypeConverter<int32_t>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, int32_t *outValue)
            {
                if (inValue->IsInt32())
                {
                    *outValue = inValue.As<v8::Int32>()->Value();
                    return true;
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<uint32_t>::To(v8::Isolate *inIsolate, uint32_t inValue)
            {
                return v8::Integer::NewFromUnsigned(inIsolate, inValue).As<v8::Value>();
            }

            bool V8TypeConverter<uint32_t>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, uint32_t *outValue)
            {
                if (inValue->IsUint32())
                {
                    *outValue = inValue.As<v8::Uint32>()->Value();
                    return true;
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<int64_t>::To(v8::Isolate *inIsolate, int64_t inValue)
            {
                return v8::Number::New(inIsolate, static_cast<double>(inValue)).As<v8::Value>();
            }

            bool V8TypeConverter<int64_t>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, int64_t *outValue)
            {
                if (inValue->IsNumber())
                {
                    return FromMaybe(inValue->IntegerValue(inIsolate->GetCurrentContext()), outValue);
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<uint64_t>::To(v8::Isolate *inIsolate, uint64_t inValue)
            {
                return v8::Number::New(inIsolate, static_cast<double>(inValue)).As<v8::Value>();
            }

            bool V8TypeConverter<uint64_t>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, uint64_t *outValue)
            {
                if (inValue->IsNumber())
                {
                    return FromMaybe(inValue->IntegerValue(inIsolate->GetCurrentContext()), outValue);
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<float>::To(v8::Isolate *inIsolate, float inValue)
            {
                return v8::Number::New(inIsolate, inValue).As<v8::Value>();
            }

            bool V8TypeConverter<float>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, float *outValue)
            {
                if (inValue->IsNumber())
                {
                    *outValue = static_cast<float>(inValue.As<v8::Number>()->Value());
                    return true;
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<double>::To(v8::Isolate *inIsolate, double inValue)
            {
                return v8::Number::New(inIsolate, inValue).As<v8::Value>();
            }

            bool V8TypeConverter<double>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, double *outValue)
            {
                if (inValue->IsNumber())
                {
                    *outValue = inValue.As<v8::Number>()->Value();
                    return true;
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<std::string>::To(v8::Isolate *inIsolate, const std::string &inValue)
            {
                return v8::String::NewFromUtf8(inIsolate, inValue.c_str(), v8::NewStringType::kNormal, inValue.length()).ToLocalChecked();
            }

            bool V8TypeConverter<std::string>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, std::string *outValue)
            {
                if (inValue->IsString())
                {
                    v8::Local<v8::String> v8Str = v8::Local<v8::String>::Cast(inValue);
                    int length = v8Str->Utf8Length(inIsolate);
                    outValue->resize(length);
                    v8Str->WriteUtf8(inIsolate, &(*outValue)[0], length, nullptr, v8::String::NO_NULL_TERMINATION);
                    return true;
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<std::u16string>::To(v8::Isolate *inIsolate, const std::u16string &inValue, v8::NewStringType inStringType)
            {
                return v8::String::NewFromTwoByte(inIsolate, reinterpret_cast<const uint16_t *>(inValue.c_str()), inStringType, inValue.length()).ToLocalChecked();
            }

            bool V8TypeConverter<std::u16string>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, std::u16string *outValue)
            {
                if (inValue->IsString())
                {
                    v8::Local<v8::String> v8Str = v8::Local<v8::String>::Cast(inValue);
                    outValue->resize(v8Str->Length());
                    v8Str->Write(inIsolate, reinterpret_cast<uint16_t *>(&(*outValue)[0]), 0, v8Str->Length(), v8::String::NO_NULL_TERMINATION);
                    return true;
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<v8::Local<v8::Function>>::To(v8::Isolate *inIsolate, v8::Local<v8::Function> inValue)
            {
                return inValue.As<v8::Value>();
            }

            bool V8TypeConverter<v8::Local<v8::Function>>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Function> *outValue)
            {
                if (inValue->IsFunction())
                {
                    *outValue = v8::Local<v8::Function>::Cast(inValue);
                    return true;
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<v8::Local<v8::Object>>::To(v8::Isolate *inIsolate, v8::Local<v8::Object> inValue)
            {
                return inValue.As<v8::Value>();
            }

            bool V8TypeConverter<v8::Local<v8::Object>>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Object> *outValue)
            {
                if (inValue->IsObject())
                {
                    *outValue = v8::Local<v8::Object>::Cast(inValue);
                    return true;
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<v8::Local<v8::Promise>>::To(v8::Isolate *inIsolate, v8::Local<v8::Promise> inValue)
            {
                return inValue.As<v8::Value>();
            }

            bool V8TypeConverter<v8::Local<v8::Promise>>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Promise> *outValue)
            {
                if (inValue->IsPromise())
                {
                    *outValue = v8::Local<v8::Promise>::Cast(inValue);
                    return true;
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<v8::Local<v8::ArrayBuffer>>::To(v8::Isolate *inIsolate, v8::Local<v8::ArrayBuffer> inValue)
            {
                return inValue.As<v8::Value>();
            }

            bool V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::ArrayBuffer> *outValue)
            {
                if (inValue->IsArrayBuffer())
                {
                    *outValue = v8::Local<v8::ArrayBuffer>::Cast(inValue);
                    return true;
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<v8::Local<v8::External>>::To(v8::Isolate *inIsolate, v8::Local<v8::External> inValue)
            {
                return inValue.As<v8::Value>();
            }

            bool V8TypeConverter<v8::Local<v8::External>>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::External> *outValue)
            {
                if (inValue->IsExternal())
                {
                    *outValue = v8::Local<v8::External>::Cast(inValue);
                    return true;
                }
                return false;
            }

            v8::Local<v8::Value> V8TypeConverter<v8::Local<v8::Value>>::To(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue)
            {
                return inValue;
            }

            bool V8TypeConverter<v8::Local<v8::Value>>::From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Value> *outValue)
            {
                *outValue = inValue;
                return true;
            }
       }
    }
}