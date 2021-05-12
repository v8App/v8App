// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_FUNCTION_TEMPLATE_H__
#define __V8_FUNCTION_TEMPLATE_H__

#include <functional>

#include "v8.h"
#include "Logging/LogMacros.h"
#include "Utils/CallbackWrapper.h"
#include "JSRuntime.h"
#include "V8Arguments.h"

namespace v8App
{
    using Utils::CallbackWrapper;

    namespace JSRuntime
    {
        namespace CppBridge
        {

             class CallbackHolderBase
            {
            public:
                v8::Local<v8::External> GetExternalHandle(IsolateWeakPtr inIsolate);

            protected:
                explicit CallbackHolderBase(IsolateWeakPtr inIsolate);
                virtual ~CallbackHolderBase();

            private:
                static void FirstWeakCallback(const v8::WeakCallbackInfo<CallbackHolderBase> &info);
                static void SecondWeakCallback(const v8::WeakCallbackInfo<CallbackHolderBase> &info);

                v8::Global<v8::External> m_ExHolder;

                CallbackHolderBase(const CallbackHolderBase &) = delete;
                CallbackHolderBase &operator=(const CallbackHolderBase &) = delete;
            };

            template <typename Signature>
            class CallbackHolder : public CallbackHolderBase
            {
            public:
                CallbackHolder(IsolateWeakPtr inIsolate, Utils::CallbackWrapper<Signature> inCallback, const char *inTypeName)
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
                LocalType value;

                if (inArgs->GetNextArg(&value) == false)
                {
                    ThrowConversionError(inArgs, inArgs->GetNextArgIndex() - 1, isMemberFunction);
                }
                return value;
            }

            template <typename Signature>
            struct CallbackDispatcher;

            template <typename R, typename... Args>
            struct CallbackDispatcher<R (*)(Args...)>
            {
                using Functor = R (*)(Args...);
                static void DispatchCallback(V8Arguments *inArgs)
                {

                    v8::Local<v8::External> v8Holder;
                    CHECK_EQ(true, inArgs->GetData(&v8Holder));
                    CallbackHolderBase *holderBase = reinterpret_cast<CallbackHolderBase *>(v8Holder->Value());

                    using HolderInstType = CallbackHolder<Functor>;
                    HolderInstType *holder = static_cast<HolderInstType *>(holderBase);

                    InvokeCallback(inArgs, holder->m_Callback,
                                   std::move(ConvertArgument<Args>(inArgs, false))...);
                }

                static void V8CallbackForFunction(const v8::FunctionCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    DispatchCallback(&args);
                }

                static void V8CallbackForProperty(const v8::PropertyCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    DispatchCallback(&args);
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


            template <typename R, typename C, typename... Args>
            struct CallbackDispatcher<R (C::*)(Args...)>
            {
                using Functor = R (C::*)(Args...);
                static void DispatchCallback(V8Arguments *inArgs)
                {

                    v8::Local<v8::External> v8Holder;
                    CHECK_EQ(true, inArgs->GetData(&v8Holder));
                    CallbackHolderBase *holderBase = reinterpret_cast<CallbackHolderBase *>(v8Holder->Value());

                    using HolderInstType = CallbackHolder<Functor>;
                    HolderInstType *holder = static_cast<HolderInstType *>(holderBase);

                    InvokeCallback(inArgs, holder->m_Callback,
                                   std::move(ConvertArgument<Args>(inArgs, true))...);
                }

                static void V8CallbackForFunction(const v8::FunctionCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    DispatchCallback(&args);
                }

                static void V8CallbackForProperty(const v8::PropertyCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    DispatchCallback(&args);
                }

            private:
                static void InvokeCallback(V8Arguments *inArgs, CallbackWrapper<Functor> &inCallback, Args... args)
                {
                    if (inArgs->NoConversionErrors())
                    {
                        C object;
                        if(inArgs->GetHolder(&object) == false)
                        {
                            inArgs->ThrowTypeError("");
                            return;
                        }

                        if constexpr (std::is_same<void, R>::value)
                        {
                            inCallback.Invoke(&object, std::move(args)...);
                        }
                        else
                        {
                            if (inArgs->Return(inCallback.Invoke(&object, std::move(args)...)) == false)
                            {
                                inArgs->ThrowTypeError("");
                            }
                        }
                    }
                }
            };

            template <typename R, typename C, typename... Args>
            struct CallbackDispatcher<R (C::*)(Args...) const>
            {
                using Functor = R (C::*)(Args...) const;
                static void DispatchCallback(V8Arguments *inArgs)
                {

                    v8::Local<v8::External> v8Holder;
                    CHECK_EQ(true, inArgs->GetData(&v8Holder));
                    CallbackHolderBase *holderBase = reinterpret_cast<CallbackHolderBase *>(v8Holder->Value());

                    using HolderInstType = CallbackHolder<Functor>;
                    HolderInstType *holder = static_cast<HolderInstType *>(holderBase);

                    InvokeCallback(inArgs, holder->m_Callback,
                                   std::move(ConvertArgument<Args>(inArgs, true))...);
                }

                static void V8CallbackForFunction(const v8::FunctionCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    DispatchCallback(&args);
                }

                static void V8CallbackForProperty(const v8::PropertyCallbackInfo<v8::Value> &info)
                {
                    V8Arguments args(info);
                    DispatchCallback(&args);
                }

            private:
                static void InvokeCallback(V8Arguments *inArgs, CallbackWrapper<Functor> &inCallback, Args... args)
                {
                    if (inArgs->NoConversionErrors())
                    {
                        C object;
                        if(inArgs->GetHolder(&object) == false)
                        {
                            inArgs->ThrowTypeError("");
                            return;
                        }

                        if constexpr (std::is_same<void, R>::value)
                        {
                            inCallback.Invoke(&object, std::move(args)...);
                        }
                        else
                        {
                            if (inArgs->Return(inCallback.Invoke(&object, std::move(args)...)) == false)
                            {
                                inArgs->ThrowTypeError("");
                            }
                        }
                    }
                }
            };

            template <typename Signature>
            v8::Local<v8::FunctionTemplate> CreateFunctionTemplate(IsolateWeakPtr inIsolate, CallbackWrapper<Signature> inCallback,
                                                                   const char *TypeName = "")
            {
                CHECK_EQ(false, inIsolate.expired());
                using HolderInstType = CallbackHolder<Signature>;
                HolderInstType *holder = new HolderInstType(inIsolate, std::move(inCallback), TypeName);
                v8::Isolate *isolate = inIsolate.lock().get();
                
                JSRuntime *runtime = JSRuntime::GetRuntime(isolate);
                CHECK_NE(nullptr, runtime);

                v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New(
                    isolate,
                    &CallbackDispatcher<Signature>::V8CallbackForFunction,
                    ConvertToV8<v8::Local<v8::External>>(isolate,
                                                         holder->GetExternalHandle(inIsolate)));

                tmpl->RemovePrototype();

                //register the external reference
                runtime->GetExternalRegistry().Register((void*)&CallbackDispatcher<Signature>::V8CallbackForFunction);

                return tmpl;
            }
        } // namespace CppBridge
    }     // namespace JSRuntime
} // namespace v8App
#endif