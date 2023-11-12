// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __CALLBACK_WRAPPER_H__
#define __CALLBACK_WRAPPER_H__

#include <functional>
#include <type_traits>
#include <memory>

#include <Logging/LogMacros.h>

/**
 * CallbackWrapper is a set of template classes desgiend to wrap function pointers.
 * Class Member functions, lambdas and std::functions.
 * Class member function can wrap static, const and non-const memeber functions
 */
namespace v8App
{
    namespace Utils
    {

        class CallbackWrapperBase
        {
        public:
            virtual bool IsMemberFunction() const { return false; }
            virtual bool IsLambdaFunction() const { return false; }
            virtual bool IsStdFunction() const { return false; }
            virtual bool IsVoid() const { return true; }
        };

        template <typename Signature, bool Lambda = false>
        class CallbackWrapper;
        /**
         * Handles free functions and static member functions
         */
        template <typename R, typename... Args>
        class CallbackWrapper<R (*)(Args...)> : public CallbackWrapperBase
        {
        public:
            using Functor = R(Args...);
            using FunctionPtr = R (*)(Args...);

            CallbackWrapper(Functor inCallback) : m_Callback(inCallback) {}
            CallbackWrapper(const CallbackWrapper &inCallback) : m_Callback(inCallback.m_Callback) {}
            CallbackWrapper(CallbackWrapper &&inCallback)
            {
                m_Callback = inCallback.m_Callback;
                inCallback.m_Callback = nullptr;
            }
            CallbackWrapper &operator=(CallbackWrapper &&inCallback) noexcept
            {
                m_Callback = std::move(inCallback.m_Callback);
                return *this;
            }

            CallbackWrapper &operator=(const CallbackWrapper &inCallback)
            {
                m_Callback = inCallback.m_Callback;
                return *this;
            }

            R Invoke(Args... args)
            {
                return std::invoke(m_Callback, std::forward<Args>(args)...);
            }

            bool IsVoid() const override
            {
                return std::is_same<void, R>::value;
            }

        private:
            FunctionPtr m_Callback = nullptr;
        };

        /**
         * Handles Member functions
         */
        template <typename R, typename C, typename... Args>
        class CallbackWrapper<R (C::*)(Args...)> : public CallbackWrapperBase
        {
        public:
            using Functor = R (C::*)(Args...);

            CallbackWrapper(Functor inCallback) : m_Callback(inCallback) {}
            CallbackWrapper(const CallbackWrapper &inCallback) : m_Callback(inCallback.m_Callback) {}
            CallbackWrapper(CallbackWrapper &&inCallback)
            {
                m_Callback = inCallback.m_Callback;
                inCallback.m_Callback = nullptr;
            }
            CallbackWrapper &operator=(const CallbackWrapper &inCallback)
            {
                m_Callback = inCallback.m_Callback;
                return *this;
            }
            CallbackWrapper &operator=(CallbackWrapper &&inCallback) noexcept
            {
                m_Callback = std::move(inCallback.m_Callback);
                return *this;
            }

            R Invoke(C *inObject, Args... args)
            {
                return std::invoke(m_Callback, inObject, std::forward<Args>(args)...);
            }

            R Invoke(std::weak_ptr<C> &inObject, Args... args)
            {
                C *obj = inObject.lock().get();
                return std::invoke(m_Callback, obj, std::forward<Args>(args)...);
            }

            bool IsMemberFunction() const override { return true; }

            bool IsVoid() const override
            {
                return std::is_same<void, R>::value;
            }

        private:
            Functor m_Callback = nullptr;
        };

        /**
         * Handles const Member functions
         */
        template <typename R, typename C, typename... Args>
        class CallbackWrapper<R (C::*)(Args...) const> : public CallbackWrapperBase
        {
        public:
            using Functor = R (C::*)(Args...) const;

            CallbackWrapper(Functor inCallback) : m_Callback(inCallback) {}
            CallbackWrapper(const CallbackWrapper &inCallback) : m_Callback(inCallback.m_Callback) {}
            CallbackWrapper(CallbackWrapper &&inCallback)
            {
                m_Callback = inCallback.m_Callback;
                inCallback.m_Callback = nullptr;
            }
            CallbackWrapper &operator=(const CallbackWrapper &inCallback)
            {
                m_Callback = inCallback.m_Callback;
                return *this;
            }
            CallbackWrapper &operator=(CallbackWrapper &&inCallback) noexcept
            {
                m_Callback = std::move(inCallback.m_Callback);
                return *this;
            }

            R Invoke(C *inObject, Args... args)
            {
                return std::invoke(m_Callback, inObject, std::forward<Args>(args)...);
            }

            R Invoke(std::weak_ptr<C> &inObject, Args... args)
            {
                C *obj = inObject.lock().get();
                return std::invoke(m_Callback, obj, std::forward<Args>(args)...);
            }

            bool IsMemberFunction() const override { return true; }

            bool IsVoid() const override
            {
                return std::is_same<void, R>::value;
            }

        private:
            Functor m_Callback = nullptr;
        };

        /**
         * Handles trivially copyable lamdba functions
         */
        template <typename R, typename C, typename... Args>
        class CallbackWrapper<R (C::*)(Args...), true> : public CallbackWrapperBase
        {
        public:
            using Functor = R (C::*)(Args...) const;

            CallbackWrapper(C &inCallback) : m_Lambda(inCallback) {}
            CallbackWrapper(const CallbackWrapper &inCallback)
                : m_Lambda(inCallback.m_Lambda)
            {
                static_assert(std::is_trivially_copyable<C>::value, "Only trivialy copyable lambda are supported");
            }
            CallbackWrapper(CallbackWrapper &&inCallback) : m_Lambda(inCallback.m_Lambda) {}

            R Invoke(Args... args)
            {
                return m_Lambda.operator()(args...);
            }

            bool IsLambdaFunction() const override { return true; }

            bool IsVoid() const override
            {
                return std::is_same<void, R>::value;
            }

            CallbackWrapper &operator=(const CallbackWrapper &inCallback) = delete;
            CallbackWrapper &operator=(CallbackWrapper &&inCallback) noexcept = delete;

        private:
            C m_Lambda;
        };

        template <typename R, typename C, typename... Args>
        class CallbackWrapper<R (C::*)(Args...) const, true> : public CallbackWrapperBase
        {
        public:
            using Functor = R (C::*)(Args...) const;

            CallbackWrapper(C &inCallback) : m_Lambda(inCallback) {}
            CallbackWrapper(const CallbackWrapper &inCallback)
                : m_Lambda(inCallback.m_Lambda)
            {
                static_assert(std::is_trivially_copyable<C>::value, "Only trivialy copyable lambda are supported");
            }
            CallbackWrapper(CallbackWrapper &&inCallback) : m_Lambda(inCallback.m_Lambda) {}
            R Invoke(Args... args)
            {
                return m_Lambda.operator()(args...);
            }

            bool IsLambdaFunction() const override { return true; }

            bool IsVoid() const override
            {
                return std::is_same<void, R>::value;
            }

            CallbackWrapper &operator=(const CallbackWrapper &inCallback) = delete;
            CallbackWrapper &operator=(CallbackWrapper &&inCallback) noexcept = delete;

        private:
            C m_Lambda;
        };

        /**
         * Handles std::function classes
         */
        template <typename R, typename... Args>
        class CallbackWrapper<std::function<R(Args...)>> : public CallbackWrapperBase
        {
            using Functor = std::function<R(Args...)>;

        public:
            CallbackWrapper(Functor inCallback) : m_Callback(inCallback) {}
            CallbackWrapper(const CallbackWrapper &inCallback) : m_Callback(inCallback.m_Callback) {}
            CallbackWrapper(CallbackWrapper &&inCallback) noexcept : m_Callback(std::move(inCallback.m_Callback)) {}

            CallbackWrapper &operator=(const CallbackWrapper &inCallback)
            {
                m_Callback = inCallback.m_Callback;
                return *this;
            }
            CallbackWrapper &operator=(const CallbackWrapper &&inCallback) noexcept
            {
                m_Callback = std::move(inCallback.m_Callback);
                return *this;
            }

            R Invoke(Args... args)
            {
                return m_Callback(std::forward<Args>(args)...);
            }

            bool IsStdFunction() const override { return true; }

            bool IsVoid() const override
            {
                return std::is_same<void, R>::value;
            }

        private:
            Functor m_Callback;
        };

        /**
         * Helper struct to tease out the function signature of a lamnda
         */
        template <typename Lambda>
        struct LambdaSignature
        {
            using type = Lambda;
        };

        template <typename C, typename R, typename... Args>
        struct LambdaSignature<R (C::*)(Args...)>
        {
            using type = R (C::*)(Args...);
        };

        /**
         * Helper function to create a callback by just passing the function
         */
        template <typename Signature>
        auto MakeCallback(Signature inCallback)
        {
            return CallbackWrapper<Signature>(inCallback);
        }

        /**
         * Helper function to create a callback for a lmabda function 
         */
        template <typename Signature>
        auto MakeCallbackForLambda(Signature inCallback)
        {
            return CallbackWrapper<typename LambdaSignature<decltype(&Signature::operator())>::type, true>(inCallback);
        }
    }
}

#endif
