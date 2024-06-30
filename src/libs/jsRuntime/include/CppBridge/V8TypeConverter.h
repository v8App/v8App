// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_TYPE_CONVERTER_H__
#define __V8_TYPE_CONVERTER_H__

#include <string>

#include "Logging/LogMacros.h"

#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            template <typename T, typename Enable = void>
            struct V8TypeConverter
            {
            };

            // Used for when the converter's to may throw
            // Create a specializtion for a type when the
            // conversion to v8 returns a Maybe value
            template <typename T>
            concept ToReturnsMaybe = requires(V8Isolate * inIsolate, T value) {
                {
                    V8TypeConverter<T>(inIsolate, value)
                } -> std::same_as<V8MBLValue>;
            };

            template <>
            struct V8TypeConverter<bool>
            {
                static V8LValue To(V8Isolate *inIsolate, bool inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, bool *outValue);
            };

            template <>
            struct V8TypeConverter<int32_t>
            {
                static V8LValue To(V8Isolate *inIsolate, int32_t inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, int32_t *outValue);
            };

            template <>
            struct V8TypeConverter<uint32_t>
            {
                static V8LValue To(V8Isolate *inIsolate, uint32_t inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, uint32_t *outValue);
            };

            template <>
            struct V8TypeConverter<int64_t>
            {
                static V8LValue To(V8Isolate *inIsolate, int64_t inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, int64_t *outValue);
            };

            template <>
            struct V8TypeConverter<uint64_t>
            {
                static V8LValue To(V8Isolate *inIsolate, uint64_t inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, uint64_t *outValue);
            };

            template <>
            struct V8TypeConverter<float>
            {
                static V8LValue To(V8Isolate *inIsolate, float inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, float *outValue);
            };

            template <>
            struct V8TypeConverter<double>
            {
                static V8LValue To(V8Isolate *inIsolate, double inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, double *outValue);
            };

            template <>
            struct V8TypeConverter<std::string>
            {
                static V8LValue To(V8Isolate *inIsolate, const std::string &inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, std::string *outValue);
            };

            template <>
            struct V8TypeConverter<std::u16string>
            {
                static V8LValue To(V8Isolate *inIsolate, const std::u16string &inValue, v8::NewStringType inStringType = v8::NewStringType::kNormal);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, std::u16string *outValue);
            };

            template <>
            struct V8TypeConverter<V8LFunction>
            {
                static V8LValue To(V8Isolate *inIsolate, V8LFunction inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, V8LFunction *outValue);
            };

            template <>
            struct V8TypeConverter<V8LObject>
            {
                static V8LValue To(V8Isolate *inIsolate, V8LObject inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, V8LObject *outValue);
            };

            template <>
            struct V8TypeConverter<V8LPromise>
            {
                static V8LValue To(V8Isolate *inIsolate, V8LPromise inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, V8LPromise *outValue);
            };

            template <>
            struct V8TypeConverter<V8LBigInt>
            {
                static V8LValue To(V8Isolate *inIsolate, V8LBigInt inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, V8LBigInt *outValue);
            };

            template <>
            struct V8TypeConverter<V8LExternal>
            {
                static V8LValue To(V8Isolate *inIsolate, V8LExternal inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, V8LExternal *outValue);
            };

            template <>
            struct V8TypeConverter<V8LValue>
            {
                static V8LValue To(V8Isolate *inIsolate, V8LValue inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, V8LValue *outValue);
            };

            template <>
            struct V8TypeConverter<V8LNumber>
            {
                static V8LValue To(V8Isolate *inIsolate, V8LNumber inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, V8LNumber *outValue);
            };

            template <>
            struct V8TypeConverter<V8LArrayBuffer>
            {
                static V8LValue To(V8Isolate *inIsolate, V8LArrayBuffer inValue);
                static bool From(V8Isolate *inIsolate, V8LValue inValue, V8LArrayBuffer *outValue);
            };

            template <typename T>
            struct V8TypeConverter<std::vector<T>>
            {
                static std::conditional_t<ToReturnsMaybe<T>,
                                          V8MBLValue,
                                          V8LValue>
                To(V8Isolate *inIsolate, std::vector<T> &inValue)
                {
                    V8LContext context = inIsolate->GetCurrentContext();
                    V8LArray array = V8Array::New(inIsolate, static_cast<int>(inValue.size()));
                    for (uint32_t x = 0; x < inValue.size(); x++)
                    {
                        V8MBLValue maybe = V8TypeConverter<T>::To(inIsolate, inValue[x]);
                        V8LValue element;
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
                static bool From(V8Isolate *inIsolate, V8LValue inValue, std::vector<T> *outValue)
                {
                    if (inValue->IsArray() == false)
                    {
                        return false;
                    }
                    std::vector<T> vector;
                    V8LArray array(V8LArray::Cast(inValue));
                    uint32_t length = array->Length();
                    V8LContext context = inIsolate->GetCurrentContext();

                    for (uint32_t x = 0; x < length; x++)
                    {
                        V8LValue v8_element;
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
            struct V8TypeConverter<v8::LocalVector<T>>
            {
                static std::conditional_t<ToReturnsMaybe<v8::Local<T>>,
                                          V8MBLValue,
                                          V8LValue>
                To(V8Isolate *inIsolate, v8::LocalVector<T> &inValue)
                {
                    V8LContext context = inIsolate->GetCurrentContext();
                    V8LArray array = V8Array::New(inIsolate, static_cast<int>(inValue.size()));
                    for (uint32_t x = 0; x < inValue.size(); x++)
                    {
                        V8MBLValue maybe = V8TypeConverter<v8::Local<T>>::To(inIsolate, inValue[x]);
                        V8LValue element;
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
                static bool From(V8Isolate *inIsolate, V8LValue inValue, v8::LocalVector<T> *outValue)
                {
                    if (inValue->IsArray() == false)
                    {
                        return false;
                    }
                    v8::LocalVector<T> vector(inIsolate);
                    V8LArray array(V8LArray::Cast(inValue));
                    uint32_t length = array->Length();
                    V8LContext context = inIsolate->GetCurrentContext();

                    for (uint32_t x = 0; x < length; x++)
                    {
                        V8LValue v8_element;
                        if (array->Get(context, x).ToLocal(&v8_element) == false)
                        {
                            return false;
                        }
                        T element;
                        if (V8TypeConverter<v8::Local<T>>::From(inIsolate, v8_element, &element) == false)
                        {
                            return false;
                        }
                        vector.push_back(element);
                    }

                    outValue->swap(vector);
                    return true;
                }
            };

            /**
             * Conveince function to deduce T
             */
            template <typename T>
            std::conditional_t<ToReturnsMaybe<T>,
                               V8MBLValue,
                               V8LValue>
            ConvertToV8(V8Isolate *inIsolate, const T &inValue)
            {
                return V8TypeConverter<T>::To(inIsolate, inValue);
            }

            template <typename T>
            bool TryConvertToV8(V8Isolate *isolate,
                                const T &input,
                                V8LValue *output)
            {
                if constexpr (ToReturnsMaybe<T>)
                {
                    return ConvertToV8(isolate, input).ToLocal(output);
                }
                else
                {
                    *output = ConvertToV8(isolate, input);
                    return true;
                }
            }

            template <typename T>
            bool ConvertFromV8(V8Isolate *inIsolate, V8LValue inValue, T *outValue)
            {
                DCHECK_NE(nullptr, inIsolate);
                return V8TypeConverter<T>::From(inIsolate, inValue, outValue);
            }
        } // namespace CppBridge
    } // namespace JSRuntime
} // namespace v8App

#endif