// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_TYPE_CONVERTER_H__
#define __V8_TYPE_CONVERTER_H__

#include <string>
#include "v8.h"
#include "Logging/LogMacros.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            //Used for when the converter's to may throw
            //Create a specializtion for a type when the
            //conversion to v8 returns a Maybe value
            template <typename T, typename Enable = void>
            struct ToReturnsMaybe
            {
                static const bool Value = false;
            };

            template <typename T, typename Enable = void>
            struct V8TypeConverter
            {
            };

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

            template <>
            struct V8TypeConverter<bool>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, bool inValue)
                {
                    return v8::Boolean::New(inIsolate, inValue).As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, bool *outValue)
                {
                    *outValue = inValue->BooleanValue(inIsolate);
                    return true;
                }
            };

            template <>
            struct V8TypeConverter<int32_t>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, int32_t inValue)
                {
                    return v8::Integer::New(inIsolate, inValue).As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, int32_t *outValue)
                {
                    if (inValue->IsInt32())
                    {
                        *outValue = inValue.As<v8::Int32>()->Value();
                        return true;
                    }
                    return false;
                }
            };

            template <>
            struct V8TypeConverter<uint32_t>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, uint32_t inValue)
                {
                    return v8::Integer::NewFromUnsigned(inIsolate, inValue).As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, uint32_t *outValue)
                {
                    if (inValue->IsUint32())
                    {
                        *outValue = inValue.As<v8::Uint32>()->Value();
                        return true;
                    }
                    return false;
                }
            };

            template <>
            struct V8TypeConverter<int64_t>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, int64_t inValue)
                {
                    return v8::Number::New(inIsolate, static_cast<double>(inValue)).As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, int64_t *outValue)
                {
                    if (inValue->IsNumber())
                    {
                        return FromMaybe(inValue->IntegerValue(inIsolate->GetCurrentContext()), outValue);
                    }
                    return false;
                }
            };

            template <>
            struct V8TypeConverter<uint64_t>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, uint64_t inValue)
                {
                    return v8::Number::New(inIsolate, static_cast<double>(inValue)).As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, uint64_t *outValue)
                {
                    if (inValue->IsNumber())
                    {
                        return FromMaybe(inValue->IntegerValue(inIsolate->GetCurrentContext()), outValue);
                    }
                    return false;
                }
            };

            template <>
            struct V8TypeConverter<float>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, float inValue)
                {
                    return v8::Number::New(inIsolate, inValue).As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, float *outValue)
                {
                    if (inValue->IsNumber())
                    {
                        *outValue = static_cast<float>(inValue.As<v8::Number>()->Value());
                        return true;
                    }
                    return false;
                }
            };

            template <>
            struct V8TypeConverter<double>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, double inValue)
                {
                    return v8::Number::New(inIsolate, inValue).As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, double *outValue)
                {
                    if (inValue->IsNumber())
                    {
                        *outValue = inValue.As<v8::Number>()->Value();
                        return true;
                    }
                    return false;
                }
            };

            template <>
            struct V8TypeConverter<std::string>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, const std::string &inValue)
                {
                    return v8::String::NewFromUtf8(inIsolate, inValue.c_str(), v8::NewStringType::kNormal, inValue.length()).ToLocalChecked();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, std::string *outValue)
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
            };

            template <>
            struct V8TypeConverter<std::wstring>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, const std::wstring &inValue, v8::NewStringType inStringType = v8::NewStringType::kNormal)
                {
                    int charSize = sizeof(std::wstring::value_type);
                    if (charSize == 2)
                    {
                        return v8::String::NewFromTwoByte(inIsolate, reinterpret_cast<const uint16_t *>(inValue.c_str()), inStringType, inValue.length()).ToLocalChecked();
                    }
                    //When possibly converting to utf 16 we'll need to make this * charSize/2 < 1 ? 1: charSize/2
                    uint16_t *converted = new uint16_t[inValue.length() + 1];
                    for (size_t x = 0; x < inValue.length(); x++)
                    {
                        //todo detect if values are > 2 bytes and convert to utf 16
                        converted[x] = static_cast<uint16_t>(inValue[x]);
                    }
                    converted[inValue.length()] = '\0';
                    v8::MaybeLocal<v8::String> retValue = v8::String::NewFromTwoByte(inIsolate, converted, inStringType, inValue.length());
                    delete[] converted;
                    return retValue.ToLocalChecked();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, std::wstring *outValue)
                {
                    if (inValue->IsString() == false)
                    {
                        return false;
                    }
                    int charSize = sizeof(std::wstring::value_type);
                    v8::Local<v8::String> v8Str = v8::Local<v8::String>::Cast(inValue);
                    int length = v8Str->Length();
                    outValue->resize(length);
                    if (charSize == 2)
                    {
                        v8Str->Write(inIsolate, reinterpret_cast<uint16_t *>(&(*outValue)[0]), 0, length, v8::String::NO_NULL_TERMINATION);
                        return true;
                    }
                    //some other byte size for wchar_t so conert it to a uint16_t first
                    uint16_t *converted = new uint16_t[length];
                    v8Str->Write(inIsolate, converted, 0, length, v8::String::NO_NULL_TERMINATION);
                    //no copy in the characters
                    for (size_t x = 0; x < length; x++)
                    {
                        (*outValue)[x] = converted[x];
                    }
                    return true;
                }
            };

            template <>
            struct V8TypeConverter<std::u16string>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, const std::u16string &inValue)
                {
                    return v8::String::NewFromTwoByte(inIsolate, reinterpret_cast<const uint16_t *>(inValue.c_str()), v8::NewStringType::kNormal, inValue.length()).ToLocalChecked();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, std::u16string *outValue)
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
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::Function>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::Function> inValue)
                {
                    return inValue.As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Function> *outValue)
                {
                    if (inValue->IsFunction())
                    {
                        *outValue = v8::Local<v8::Function>::Cast(inValue);
                        return true;
                    }
                    return false;
                }
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::Object>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::Object> inValue)
                {
                    return inValue.As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Object> *outValue)
                {
                    if (inValue->IsObject())
                    {
                        *outValue = v8::Local<v8::Object>::Cast(inValue);
                        return true;
                    }
                    return false;
                }
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::Promise>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::Promise> inValue)
                {
                    return inValue.As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Promise> *outValue)
                {
                    if (inValue->IsPromise())
                    {
                        *outValue = v8::Local<v8::Promise>::Cast(inValue);
                        return true;
                    }
                    return false;
                }
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::ArrayBuffer>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::ArrayBuffer> inValue)
                {
                    return inValue.As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::ArrayBuffer> *outValue)
                {
                    if (inValue->IsArrayBuffer())
                    {
                        *outValue = v8::Local<v8::ArrayBuffer>::Cast(inValue);
                        return true;
                    }
                    return false;
                }
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::External>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::External> inValue)
                {
                    return inValue.As<v8::Value>();
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::External> *outValue)
                {
                    if (inValue->IsExternal())
                    {
                        *outValue = v8::Local<v8::External>::Cast(inValue);
                        return true;
                    }
                    return false;
                }
            };

            template <>
            struct V8TypeConverter<v8::Local<v8::Value>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue)
                {
                    return inValue;
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, v8::Local<v8::Value> *outValue)
                {
                    *outValue = inValue;
                    return true;
                }
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
                            DCHECK_EQ(true, false)
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

            //Create a symbol from a std::string
            v8::Local<v8::String> CreateSymbol(v8::Isolate *inIsolate, const std::string &inString);

            //create a symbol from a std::wstring
            v8::Local<v8::String> CreateSymbol(v8::Isolate *inIsolate, const std::wstring &inString);
        } // namespace CppBridge
    }     // namespace JSRuntime
} // namespace v8App

#endif