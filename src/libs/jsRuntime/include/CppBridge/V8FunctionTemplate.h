// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_FUNCTION_TEMPLATE_H__
#define __V8_FUNCTION_TEMPLATE_H__

#include <functional>

#include "v8/v8.h"

#include "Logging/LogMacros.h"
#include "Utils/CallbackWrapper.h"
#include "JSRuntime.h"
#include "V8Arguments.h"
#include "CallbackHolderBase.h"

namespace v8App
{
    using Utils::CallbackWrapper;

    namespace JSRuntime
    {
        namespace CppBridge
        {
            template <typename Signature>
            class CallbackHolder : public CallbackHolderBase
            {
            public:
                CallbackHolder(v8::Isolate *inIsolate, Utils::CallbackWrapper<Signature> inCallback, const char *inTypeName)
                    : CallbackHolderBase(inIsolate), m_Callback(inCallback), m_TypeName(inTypeName) {}

                Utils::CallbackWrapper<Signature> m_Callback;
                const char *m_TypeName;

            private:
                ~CallbackHolder() override {}

                CallbackHolder(const CallbackHolder &) = delete;
                CallbackHolder &operator=(const CallbackHolder &) = delete;
            };

            template <typename T>
            struct CppArgumentTraits
            {
                typedef T LocalType;
            };

            template <typename T>
            struct CppArgumentTraits<const T &>
            {
                typedef T LocalType;
            };

            template <typename T>
            struct CppArgumentTraits<const T *>
            {
                typedef T *LocalType;
            };

            template <typename ArgType>
            typename CppArgumentTraits<ArgType>::LocalType ConvertArgument(V8Arguments *inArgs, bool isMemberFunction)
            {
                using LocalType = typename CppArgumentTraits<ArgType>::LocalType;

                if constexpr (std::is_same<v8::FunctionCallbackInfo<v8::Value>, LocalType>::value)
                {
                    if (inArgs->IsPropertyCallback())
                    {
                        // if it's not a FunctionCallbackInfo then we can't continue
                        CHECK_TRUE(false);
                    }
                    return inArgs->GetFunctionInfo();
                }
                else if constexpr (std::is_same<v8::PropertyCallbackInfo<v8::Value>, LocalType>::value)
                {
                    if (inArgs->IsPropertyCallback() == false)
                    {
                        // if it's not a PropertyCallbackInfo then we can't continue
                        CHECK_TRUE(false);
                    }
                    return inArgs->GetPropertyInfo();
                }
                else
                {
                    LocalType value;

                    if (inArgs->GetNextArg(&value) == false)
                    {
                        ThrowConversionError(inArgs, inArgs->GetNextArgIndex() - 1, isMemberFunction);
                    }
                    return value;
                }
            }

            template <typename Signature>
            struct CallbackDispatcher;

            // normal free or static functions
            template <typename R, typename... Args>
            struct CallbackDispatcher<R (*)(Args...)>
            {
                using Functor = R (*)(Args...);
                static void DispatchCallback(V8Arguments *inArgs, JSRuntimeSharedPtr inRuntime)
                {

                    // v8::Local<v8::External> v8Holder;
                    // CHECK_TRUE(inArgs->GetData(&v8Holder));
                    intptr_t funcAddress = reinterpret_cast<intptr_t>(&CppBridge::CallbackDispatcher<Functor>::V8CallbackForFunction);
                    CallbackHolderBase *holderBase = inRuntime->GetFunctionTemplate(funcAddress);
                    if (holderBase == nullptr)
                    {
                        return;
                    }

                    using HolderInstType = CallbackHolder<Functor>;
                    HolderInstType *holder = static_cast<HolderInstType *>(holderBase);

                    InvokeCallback(inArgs, holder->m_Callback,
                                   std::move(ConvertArgument<Args>(inArgs, false))...);
                }

                static void V8CallbackForFunction(const v8::FunctionCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    DispatchCallback(&args, runtime);
                }

                static void V8CallbackForProperty(const v8::PropertyCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    if (runtime == nullptr)
                    {
                        // should throw some sort of error
                        return;
                    }
                    DispatchCallback(&args, runtime);
                }

            private:
                static void InvokeCallback(V8Arguments *inArgs, CallbackWrapper<Functor> &inCallback, Args... args)
                {
                    if (inArgs->NoConversionErrors())
                    {
                        if constexpr (std::is_same<void, R>::value)
                        {
                            inCallback.Invoke(std::move(args)...);
                        }
                        else
                        {
                            if (inArgs->Return(inCallback.Invoke(std::move(args)...)) == false)
                            {
                                inArgs->ThrowTypeError("");
                            }
                        }
                    }
                }
            };

            // non-const member functions
            template <typename R, typename C, typename... Args>
            struct CallbackDispatcher<R (C::*)(Args...)>
            {
                using Functor = R (C::*)(Args...);
                static void DispatchCallback(V8Arguments *inArgs, JSRuntimeSharedPtr inRuntime)
                {

                    v8::Local<v8::External> v8Holder;
                    CHECK_TRUE(inArgs->GetData(&v8Holder));
                    CallbackHolderBase *holderBase = reinterpret_cast<CallbackHolderBase *>(v8Holder->Value());

                    using HolderInstType = CallbackHolder<Functor>;
                    HolderInstType *holder = static_cast<HolderInstType *>(holderBase);

                    InvokeCallback(inArgs, holder->m_Callback,
                                   std::move(ConvertArgument<Args>(inArgs, true))...);
                }

                static void V8CallbackForFunction(const v8::FunctionCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    DispatchCallback(&args, runtime);
                }

                static void V8CallbackForProperty(const v8::PropertyCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    DispatchCallback(&args, runtime);
                }

            private:
                static void InvokeCallback(V8Arguments *inArgs, CallbackWrapper<Functor> &inCallback, Args... args)
                {
                    if (inArgs->NoConversionErrors())
                    {
                        C *object;
                        if (inArgs->GetHolder(&object) == false)
                        {
                            inArgs->ThrowTypeError("");
                            return;
                        }

                        if constexpr (std::is_same<void, R>::value)
                        {
                            inCallback.Invoke(object, std::move(args)...);
                        }
                        else
                        {
                            if (inArgs->Return(inCallback.Invoke(object, std::move(args)...)) == false)
                            {
                                inArgs->ThrowTypeError("");
                            }
                        }
                    }
                }
            };
            // const member functions
            template <typename R, typename C, typename... Args>
            struct CallbackDispatcher<R (C::*)(Args...) const>
            {
                using Functor = R (C::*)(Args...) const;
                static void DispatchCallback(V8Arguments *inArgs, JSRuntimeSharedPtr inRuntime)
                {

                    v8::Local<v8::External> v8Holder;
                    CHECK_TRUE(inArgs->GetData(&v8Holder));
                    CallbackHolderBase *holderBase = reinterpret_cast<CallbackHolderBase *>(v8Holder->Value());

                    using HolderInstType = CallbackHolder<Functor>;
                    HolderInstType *holder = static_cast<HolderInstType *>(holderBase);

                    InvokeCallback(inArgs, holder->m_Callback,
                                   std::move(ConvertArgument<Args>(inArgs, true))...);
                }

                static void V8CallbackForFunction(const v8::FunctionCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    DispatchCallback(&args, runtime);
                }

                static void V8CallbackForProperty(const v8::PropertyCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    DispatchCallback(&args, runtime);
                }

            private:
                static void InvokeCallback(V8Arguments *inArgs, CallbackWrapper<Functor> &inCallback, Args... args)
                {
                    if (inArgs->NoConversionErrors())
                    {
                        C *object;
                        if (inArgs->GetHolder(&object) == false)
                        {
                            inArgs->ThrowTypeError("");
                            return;
                        }

                        if constexpr (std::is_same<void, R>::value)
                        {
                            inCallback.Invoke(object, std::move(args)...);
                        }
                        else
                        {
                            if (inArgs->Return(inCallback.Invoke(object, std::move(args)...)) == false)
                            {
                                inArgs->ThrowTypeError("");
                            }
                        }
                    }
                }
            };

            template <typename Signature>
            v8::Local<v8::FunctionTemplate> CreateFunctionTemplate(v8::Isolate *inIsolate, CallbackWrapper<Signature> inCallback,
                                                                   const char *TypeName = "", bool isConstructor = false)
            {
                using HolderInstType = CallbackHolder<Signature>;
                v8::Local<v8::FunctionTemplate> tmpl;

                HolderInstType *holder = new HolderInstType(inIsolate, std::move(inCallback), TypeName);

                JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(inIsolate);
                CHECK_NE(nullptr, runtime);

                intptr_t funcAddress = reinterpret_cast<intptr_t>(&CppBridge::CallbackDispatcher<Signature>::V8CallbackForFunction);
                runtime->SetFunctionTemplate(funcAddress, holder);
                if (isConstructor)
                {
                    // for the constrcutor function we don't need a holder object since it'll be creatig one when called
                    tmpl = v8::FunctionTemplate::New(inIsolate);
                    tmpl->SetCallHandler(&CallbackDispatcher<Signature>::V8CallbackForFunction);
                    tmpl->SetLength(1);
                }
                else
                {
                    tmpl = v8::FunctionTemplate::New(
                        inIsolate,
                        &CallbackDispatcher<Signature>::V8CallbackForFunction);

                    tmpl->RemovePrototype();
                }
                // register the external reference
                // runtime->GetExternalRegistry().Register((void *)&CallbackDispatcher<Signature>::V8CallbackForFunction);
                return tmpl;
            }
        } // namespace CppBridge
    }     // namespace JSRuntime
} // namespace v8App
#endif