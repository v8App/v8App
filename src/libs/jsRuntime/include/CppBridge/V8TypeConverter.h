// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_TYPE_CONVERTER_H__
#define __V8_TYPE_CONVERTER_H__

#include <string>

#include "v8/v8.h"

#include "Logging/LogMacros.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            // Used for when the converter's to may throw
            // Create a specializtion for a type when the
            // conversion to v8 returns a Maybe value
            template <typename T, typename Enable = void>
            struct ToReturnsMaybe
            {
                static const bool Value = false;
            };

            template <typename T, typename Enable = void>
            struct V8TypeConverter
            {
            };

            template <>
            struct V8TypeConverter<bool>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, bool inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, bool *outValue);
            };

            template <>
            struct V8TypeConverter<int32_t>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, int32_t inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, int32_t *outValue);
            };

            template <>
            struct V8TypeConverter<uint32_t>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, uint32_t inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, uint32_t *outValue);
            };

            template <>
            struct V8TypeConverter<int64_t>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, int64_t inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, int64_t *outValue);
            };

            template <>
            struct V8TypeConverter<uint64_t>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, uint64_t inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, uint64_t *outValue);
            };

            template <>
            struct V8TypeConverter<float>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, float inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, float *outValue);
            };

            template <>
            struct V8TypeConverter<double>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, double inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, double *outValue);
            };

            template <>
            struct V8TypeConverter<std::string>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, const std::string &inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, std::string *outValue);
            };

            template <>
            struct V8TypeConverter<std::u16string>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, const std::u16string &inValue, v8::NewStringType inStringType = v8::NewStringType::kNormal);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, std::u16string *outValue);
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::Function>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::Function> inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Function> *outValue);
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::Object>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::Object> inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Object> *outValue);
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::Promise>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::Promise> inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Promise> *outValue);
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::ArrayBuffer>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::ArrayBuffer> inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::ArrayBuffer> *outValue);
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::External>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::External> inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::External> *outValue);
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::Value>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Value> *outValue);
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::Number>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::Number> inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Number> *outValue);
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::BigInt>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::BigInt> inValue);
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::BigInt> *outValue);
            };

            template <typename T>
            struct ToReturnsMaybe<std::vector<T>>
            {
                static const bool Value = ToReturnsMaybe<T>::Value;
            };

            template <typename T>
            struct V8TypeConverter<std::vector<T>>
            {
                static std::conditional_t<ToReturnsMaybe<T>::Value,
                                          v8::MaybeLocal<v8::Value>,
                                          v8::Local<v8::Value>>
                To(v8::Isolate *inIsolate, std::vector<T> &inValue)
                {
                    v8::Local<v8::Context> context = inIsolate->GetCurrentContext();
                    v8::Local<v8::Array> array = v8::Array::New(inIsolate, static_cast<int>(inValue.size()));
                    for (uint32_t x = 0; x < inValue.size(); x++)
                    {
                        v8::MaybeLocal<v8::Value> maybe = V8TypeConverter<T>::To(inIsolate, inValue[x]);
                        v8::Local<v8::Value> element;
                        if (maybe.ToLocal(&element) == false)
                        {
                            return {};
                        }
                        bool created;
                        if (array->CreateDataProperty(context, x, element).To(&created) == false || created == false)
                        {
                            DCHECK_TRUE(false);
                        }
                    }
                    return array;
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, std::vector<T> *outValue)
                {
                    if (inValue->IsArray() == false)
                    {
                        return false;
                    }
                    std::vector<T> vector;
                    v8::Local<v8::Array> array(v8::Local<v8::Array>::Cast(inValue));
                    uint32_t length = array->Length();
                    v8::Local<v8::Context> context = inIsolate->GetCurrentContext();

                    for (uint32_t x = 0; x < length; x++)
                    {
                        v8::Local<v8::Value> v8_element;
                        if (array->Get(context, x).ToLocal(&v8_element) == false)
                        {
                            return false;
                        }
                        T element;
                        if (V8TypeConverter<T>::From(inIsolate, v8_element, &element) == false)
                        {
                            return false;
                        }
                        vector.push_back(element);
                    }

                    outValue->swap(vector);
                    return true;
                }
            };

            template <typename T>
            std::conditional_t<ToReturnsMaybe<T>::Value,
                               v8::MaybeLocal<v8::Value>,
                               v8::Local<v8::Value>>
            ConvertToV8(v8::Isolate *inIsolate, const T &inValue)
            {
                return V8TypeConverter<T>::To(inIsolate, inValue);
            }

            template <typename T>
            std::enable_if_t<ToReturnsMaybe<T>::Value, bool>
            TryConvertToV8(v8::Isolate *inIsolate, const T &inValue, v8::Local<v8::Value> *outValue)
            {
                return ConvertToV8(inIsolate, inValue).ToLocal(outValue);
            }

            template <typename T>
            std::enable_if_t<!ToReturnsMaybe<T>::Value, bool>
            TryConvertToV8(v8::Isolate *inIsolate, const T &inValue, v8::Local<v8::Value> *outValue)
            {
                *outValue = ConvertToV8(inIsolate, inValue);
                return true;
            }

            template <typename T>
            bool ConvertFromV8(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, T *outValue)
            {
                DCHECK_NE(nullptr, inIsolate);
                return V8TypeConverter<T>::From(inIsolate, inValue, outValue);
            }
        } // namespace CppBridge
    } // namespace JSRuntime
} // namespace v8App

#endif