// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "CppBridge/CallbackRegistry.h"
#include "CppBridge/V8FunctionTemplate.h"
#include "CppBridge/V8CppObject.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            class TestCallbackRegistry : public CallbackRegistry
            {
            public:
                ~TestCallbackRegistry()
                {
                    s_Instance.release();
                    s_Instance.reset(s_Original);
                }

                static TestCallbackRegistry *CreateTestInstance()
                {
                    if (s_Instance != nullptr)
                    {
                        s_Original = s_Instance.get();
                        s_Instance.release();
                    }
                    TestCallbackRegistry *registry = new TestCallbackRegistry();
                    s_Instance.reset(registry);
                    return registry;
                }

            protected:
                static inline CallbackRegistry *s_Original = nullptr;
            };

            class TestCallbackRegisterObj final : public CppBridge::V8CppObject<TestCallbackRegisterObj>
            {
            public:
                DEF_V8CPP_OBJ_FUNCTIONS(TestCallbackRegisterObj);
            };

            IMPL_V8CPPOBJ_DESERIALIZER(TestCallbackRegisterObj)
            {
            }

            IMPL_V8CPPOBJ_SERIALIZER(TestCallbackRegisterObj)
            {
            }

            IMPL_V8CPPOBJ_REGISTER_CLASS_FUNCS(TestCallbackRegisterObj)
            {
            }

            IMPL_V8CPPOBJ_REGISTER_CLASS_GLOBAL_TEMPLATE(TestCallbackRegisterObj)
            {
            }

            static int TestGlobalRegisterValue = 0;

            void TestNamespaceFunctionRegistration(JSContextSharedPtr inContext, V8LObject &inGlobal)
            {
                TestGlobalRegisterValue = 5;
            }

            void TestFunctionRegistration()
            {
            }

            template <typename Signature>
            intptr_t GetV8FunctionAddress(Utils::CallbackWrapper<Signature> inCallback)
            {
                return reinterpret_cast<intptr_t>(&CallbackDispatcher<Signature>::V8CallbackForFunction);
            }

            TEST(CallbackRegistryTest, TestGetReferencesRegister)
            {
                std::unique_ptr<TestCallbackRegistry> registry(TestCallbackRegistry::CreateTestInstance());
                auto callback = Utils::MakeCallback(&TestFunctionRegistration);

                CallbackRegistry::Register(Utils::MakeCallback(&TestFunctionRegistration));
                const std::vector<intptr_t> &externals = CallbackRegistry::GetReferences();
                EXPECT_EQ(2, externals.size());
                EXPECT_EQ(externals[0], GetV8FunctionAddress(Utils::MakeCallback(&TestFunctionRegistration)));

                EXPECT_EQ(nullptr, CallbackRegistry::GetCallbackHolder(0));
                EXPECT_NE(nullptr, CallbackRegistry::GetCallbackHolder(callback.GetFuncAddress()));
            }

            TEST(CallbackRegistryTest, TestNamespaceRegistrationFunction)
            {
                std::unique_ptr<TestCallbackRegistry> registry(TestCallbackRegistry::CreateTestInstance());

                CallbackRegistry::AddNamespaceSetupFunction(TestNamespaceFunctionRegistration, {"test", "test2"});
                EXPECT_EQ(0, TestGlobalRegisterValue);
                V8LObject global;

                CallbackRegistry::RunNamespaceSetupFunctions(JSContextSharedPtr(), global, "test");
                EXPECT_EQ(5, TestGlobalRegisterValue);

                TestGlobalRegisterValue = 0;
                CallbackRegistry::RunNamespaceSetupFunctions(JSContextSharedPtr(), global);
                EXPECT_EQ(0, TestGlobalRegisterValue);

                CallbackRegistry::RunNamespaceSetupFunctions(JSContextSharedPtr(), global, "test2");
                EXPECT_EQ(5, TestGlobalRegisterValue);
            }

            TEST(CallbackRegistryTest, TestRegisterGetNativeObjInfo)
            {
                std::unique_ptr<TestCallbackRegistry> registry(TestCallbackRegistry::CreateTestInstance());

                EXPECT_EQ(nullptr, CallbackRegistry::GetNativeObjectInfoFromTypeName(TestCallbackRegisterObj::s_V8CppObjInfo.m_TypeName));
                CallbackRegistry::RegisterObjectInfo(&TestCallbackRegisterObj::s_V8CppObjInfo);
                EXPECT_EQ(&TestCallbackRegisterObj::s_V8CppObjInfo, CallbackRegistry::GetNativeObjectInfoFromTypeName(TestCallbackRegisterObj::s_V8CppObjInfo.m_TypeName));
            }
        }
    }
}