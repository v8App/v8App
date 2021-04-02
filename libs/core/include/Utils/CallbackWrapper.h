// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __CALLBACK_WRAPPER_H__
#define __CALLBACK_WRAPPER_H__

#include <functional>
#include <memory>

#include <Logging/LogMacros.h>

/**
 * CallbackWrapper is a set of template classes desgiend to wrap function pointers.
 * Class Memeber functions, lambdas nd std::functions.
 * Class memeber function can bind static, const and non-const memeber functions and
 * can be bound to a specific object or be unbound and have an object passed in upon
 * invocation.
 */
namespace v8App
{
    namespace Utils
    {
        template<typename T>
        struct MemberFuncType
        {
            using type = T;
        };

        template<typename C, typename R, typename...Args>
        struct MemberFuncType<R(C::*)(Args...) const>
        {
            using type = std::function<R(Args...)>;
        };

        template <typename Signature>
        class CallbackWrapper;

        /**
         * Handles std::function classes
         */
        template <typename R, typename... Args>
        class CallbackWrapper<std::function<R(Args...)>>
        {
            using Functor = std::function<R(Args...)>;

        public:
            explicit CallbackWrapper(Functor inCallback)
            {
                m_Callback = inCallback;
            }

            R invoke(Args... args)
            {
                return m_Callback(args...);
            }

            CallbackWrapper(const CallbackWrapper&) = delete;
            CallbackWrapper* operator=(const CallbackWrapper&) = delete;
        private:
            Functor m_Callback;
        };

        /**
         * Handles Static member functions and nomrmal function pointers
         */
       template <typename R, typename... Args>
        class CallbackWrapper<R (*)(Args...)>
        {
            using Functor = R (*)(Args...);

        public:
            CallbackWrapper(Functor inCallback)
            {
                m_Callback = inCallback;
            }
            
            CallbackWrapper(CallbackWrapper&&) = default;

            R invoke(Args... args)
            {
                return std::invoke(m_Callback, args...);
            }

            CallbackWrapper(const CallbackWrapper&) = delete;
            CallbackWrapper* operator=(const CallbackWrapper&) = delete;
        private:
            Functor m_Callback;
        };


        /**
         * Memeber non const methods
         */
        template <typename R, typename... Args, typename T>
        class CallbackWrapper<R (T::*)(Args...)>
        {
            using Functor = R (T::*)(Args...);

        public:
            CallbackWrapper(Functor inCallback)
            {
                m_Callback = inCallback;
            }

            R invoke(T *inObject, Args... args)
            {
                CHECK_NOT_NULL(inObject);

                return std::invoke(m_Callback, inObject, args...);
            }

            R invoke(std::weak_ptr<T> inWeakPtr, Args... args)
            {
                T *object;

                CHECK_EQ(false, inWeakPtr.expired());
                object = inWeakPtr.lock().get();

                return std::invoke(m_Callback, object, args...);
            }

            CallbackWrapper(const CallbackWrapper&) = delete;
            CallbackWrapper* operator=(const CallbackWrapper&) = delete;
        private:
            Functor m_Callback;
        };

        /**
         * Memeber const methods
         */
        template <typename R, typename... Args, typename T>
        class CallbackWrapper<R (T::*)(Args...) const>
        {
            using Functor = R (T::*)(Args...) const;

        public:
            CallbackWrapper(Functor inCallback)
            {
                m_Callback = inCallback;
            }
            R invoke(T *inObject, Args... args)
            {
                CHECK_NOT_NULL(inObject);

                return std::invoke(m_Callback, inObject, args...);
            }

            R invoke(std::weak_ptr<T> inWeakPtr, Args... args)
            {
                T *object;

                CHECK_EQ(false, inWeakPtr.expired());
                object = inWeakPtr.lock().get();

                return std::invoke(m_Callback, object, args...);
            }

            CallbackWrapper(const CallbackWrapper&) = delete;
            CallbackWrapper* operator=(const CallbackWrapper&) = delete;
        private:
            Functor m_Callback;
        };

        /**
         * Helpers to Allow for easier creation by ust calling the helper and passing it
         * the callback without hacing to specify exact signatures
         */

        /**
         * Handles function pointers, std::functions and lambdas
         */
        template <typename Signature>
        auto MakeCallback(Signature inCallback)
        {
            return new CallbackWrapper<Signature>(inCallback);
        }

        /**
         * Converts the lambda function to a std::function
         */
        template<typename Signature>
        typename MemberFuncType<decltype(&Signature::operator())>::type
        FunctionFromLambda(Signature const &inLambda)
        {
            return inLambda;
        }

        /**
         * Handles lambdas
         */
        template <typename Signature>
        auto MakeLambdaCallback(Signature inCallback)
        {
            return new CallbackWrapper<typename MemberFuncType<decltype(&Signature::operator())>::type>(FunctionFromLambda(inCallback));
        }

        /**
         * Hnadles memeber function that are no bound to a specific instance
         */
        template <typename Signature>
        auto MakeMemberCallback(Signature inCallback)
        {
            return new CallbackWrapper<Signature>(inCallback);
        }

        /**
         * Handles static member functions could be done with the MakeCallback above
         * but this just makes it clear.
         */
        template <typename Signature>
        auto MakeStaticMemberCallback(Signature inCallback)
        {
            return new CallbackWrapper<Signature>(inCallback);
        }
    }
}

#endif
