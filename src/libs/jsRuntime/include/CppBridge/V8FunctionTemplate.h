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
#include "CppBridge/CallbackRegistry.h"

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
                CallbackHolder(Utils::CallbackWrapper<Signature> inCallback, const char *inTypeName)
                    : m_Callback(inCallback), m_TypeName(inTypeName) {}

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

                if constexpr (std::is_same<V8FuncCallInfoValue, LocalType>::value)
                {
                    if (inArgs->IsPropertyCallback())
                    {
                        // if it's not a FunctionCallbackInfo then we can't continue
                        CHECK_TRUE(false);
                    }
                    return inArgs->GetFunctionInfo();
                }
                else if constexpr (std::is_same<V8PropCallInfoValeu, LocalType>::value)
                {
                    if (inArgs->IsPropertyCallback() == false)
                    {
                        // if it's not a PropertyCallbackInfo then we can't continue
                        CHECK_TRUE(false);
                    }
                    return inArgs->GetPropertyInfo();
                }
                else if constexpr (std::is_same<V8Isolate *, LocalType>::value)
                {
                    return inArgs->GetIsolate();
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

                    V8LBigInt v8FuncAddress;
                    CHECK_TRUE(inArgs->GetData(&v8FuncAddress));
                    size_t funcAddress = reinterpret_cast<size_t>((void*)v8FuncAddress->Uint64Value());

                    using HolderInstType = CallbackHolder<Functor>;
                    HolderInstType *holder = static_cast<HolderInstType *>(CallbackRegistry::GetCallbackHolder(funcAddress));

                    if (holder == nullptr)
                    {
                        return;
                    }

                    InvokeCallback(inArgs, holder->m_Callback,
                                   std::move(ConvertArgument<Args>(inArgs, false))...);
                }

                static void V8CallbackForFunction(const V8FuncCallInfoValue &info)
                {
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    if (runtime == nullptr)
                    {
                        // should throw some sort of error
                        return;
                    }
                    V8Arguments args(info);
                    DispatchCallback(&args, runtime);
                }

                static void V8CallbackForProperty(const V8PropCallInfoValeu &info)
                {
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    if (runtime == nullptr)
                    {
                        // should throw some sort of error
                        return;
                    }
                    V8Arguments args(info);
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
                    V8LBigInt v8FuncAddress;
                    CHECK_TRUE(inArgs->GetData(&v8FuncAddress));
                    size_t funcAddress = reinterpret_cast<size_t>((void*)v8FuncAddress->Uint64Value());

                    using HolderInstType = CallbackHolder<Functor>;
                    HolderInstType *holder = static_cast<HolderInstType *>(CallbackRegistry::GetCallbackHolder(funcAddress));

                    if (holder == nullptr)
                    {
                        return;
                    }

                    InvokeCallback(inArgs, holder->m_Callback,
                                   std::move(ConvertArgument<Args>(inArgs, true))...);
                }

                static void V8CallbackForFunction(const V8FuncCallInfoValue &info)
                {
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    if (runtime == nullptr)
                    {
                        // should throw some sort of error
                        return;
                    }
                    V8Arguments args(info);
                    DispatchCallback(&args, runtime);
                }

                static void V8CallbackForProperty(const V8PropCallInfoValeu &info)
                {
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    if (runtime == nullptr)
                    {
                        // should throw some sort of error
                        return;
                    }
                    V8Arguments args(info);
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

                    V8LBigInt v8FuncAddress;
                    CHECK_TRUE(inArgs->GetData(&v8FuncAddress));
                    size_t funcAddress = reinterpret_cast<size_t>((void*)v8FuncAddress->Uint64Value());

                    using HolderInstType = CallbackHolder<Functor>;
                    HolderInstType *holder = static_cast<HolderInstType *>(CallbackRegistry::GetCallbackHolder(funcAddress));

                    if (holder == nullptr)
                    {
                        return;
                    }

                    InvokeCallback(inArgs, holder->m_Callback,
                                   std::move(ConvertArgument<Args>(inArgs, true))...);
                }

                static void V8CallbackForFunction(const V8FuncCallInfoValue &info)
                {
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    if (runtime == nullptr)
                    {
                        // should throw some sort of error
                        return;
                    }
                    V8Arguments args(info);
                    DispatchCallback(&args, runtime);
                }

                static void V8CallbackForProperty(const V8PropCallInfoValeu &info)
                {
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(info.GetIsolate());
                    if (runtime == nullptr)
                    {
                        // should throw some sort of error
                        return;
                    }
                    V8Arguments args(info);
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
            V8LFuncTpl CreateFunctionTemplate(V8Isolate *inIsolate, CallbackWrapper<Signature> inCallback,
                                                                   const char *inTypeName = "", bool isConstructor = false)
            {
                using HolderInstType = CallbackHolder<Signature>;
                V8LFuncTpl tmpl;

                size_t funcAddress = inCallback.GetFuncAddress();
                HolderInstType *holder = static_cast<HolderInstType *>(CallbackRegistry::GetCallbackHolder(funcAddress));
                if (holder == nullptr)
                {
                    CallbackRegistry::Register(inCallback, inTypeName);
                }

                JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(inIsolate);
                CHECK_NE(nullptr, runtime);

                if (isConstructor)
                {
                    // for the constrcutor function we don't need a holder object since it'll be creatig one when called
                    tmpl = V8FuncTpl::New(inIsolate);
                    tmpl->SetCallHandler(&CallbackDispatcher<Signature>::V8CallbackForFunction,
                                         V8BigInt::NewFromUnsigned(inIsolate, funcAddress));
                    tmpl->SetLength(1);
                }
                else
                {
                    tmpl = V8FuncTpl::New(
                        inIsolate,
                        &CallbackDispatcher<Signature>::V8CallbackForFunction,
                        V8BigInt::NewFromUnsigned(inIsolate, funcAddress));

                    tmpl->RemovePrototype();
                }
                return tmpl;
            }
        } // namespace CppBridge
    } // namespace JSRuntime
} // namespace v8App
#endif