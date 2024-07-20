// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "V8Fixture.h"

#include "JSSnapshotCreator.h"
#include "CppBridge/CallbackRegistry.h"
#include "JSUtilities.h"
#include "CppBridge/V8FunctionTemplate.h"
#include "CppBridge/V8CppObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"
#include "CppBridge/V8CppObjHandle.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSnapshotCreatorTest = V8Fixture;

        static std::string g_FunctionString;
        static std::string g_FunctionString2;
        void testFunctionInRuntime(std::string inString)
        {
            g_FunctionString = inString;
        }

        void testFunctionInRuntime2(std::string inString)
        {
            g_FunctionString2 = inString;
        }

        void RegisterFuncTemplate(JSRuntimeSharedPtr inRuntime, V8LFuncTpl &inGlobal)
        {
            V8Isolate *isolate = inRuntime->GetIsolate();
            auto funcTpl = CppBridge::CreateFunctionTemplate(isolate, Utils::MakeCallback(testFunctionInRuntime));
            inGlobal->Set(JSUtilities::StringToV8(isolate, "test"), funcTpl);
            funcTpl = CppBridge::CreateFunctionTemplate(isolate, Utils::MakeCallback(testFunctionInRuntime2));
            inGlobal->Set(JSUtilities::StringToV8(isolate, "test2"), funcTpl);
        }

        REGISTER_FUNCS(testFunctionInRuntime)
        {
            CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&testFunctionInRuntime));
            CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&testFunctionInRuntime2));
            CppBridge::CallbackRegistry::RegisterGlobalRegisterer(&RegisterFuncTemplate);
        }

        class TestSnapObject final : public CppBridge::V8CppObject<TestSnapObject>
        {
        public:
            virtual ~TestSnapObject()
            {
                m_Value = 0;
            }
        
            static inline TestSnapObject *snapObjInstance = nullptr;

            DEF_V8CPP_OBJ_FUNCTIONS(TestSnapObject);

            static V8LValue Constructor(V8Isolate *isolate)
            {
                JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(isolate);
                V8LContext context = isolate->GetCurrentContext();
                CppBridge::V8CppObjHandle<TestSnapObject> handle = TestSnapObject::NewObj(runtime, context);
                snapObjInstance = handle.Get();
                return handle.ToV8();
            }

            void SetValue(int inValue) { m_Value = inValue; }
            int GetValue() { return m_Value; }

        protected:
            int m_Value = 5;
        };

        IMPL_V8CPPOBJ_DESERIALIZER(TestSnapObject)
        {
            // TestSnapObject::snapObjInstance = new TestSnapObject();
            // int *value = (int *)inSerialized.data;
            // TestSnapObject::snapObjInstance->m_Value = *value;
            // return TestSnapObject::snapObjInstance;
        }

        IMPL_V8CPPOBJ_SERIALIZER(TestSnapObject)
        {
            TestSnapObject *instance = reinterpret_cast<TestSnapObject *>(inNativeObject);
            V8StartupData data;
            int *value = new int(instance->m_Value);
            data.data = (const char *)value;
            data.raw_size = sizeof(int);
            return data;
        }

        IMPL_V8CPPOBJ_REGISTER_CLASS_FUNCS(TestSnapObject)
        {
            CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestSnapObject::Constructor));
            CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestSnapObject::SetValue));
            CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestSnapObject::GetValue));
        }

        IMPL_V8CPPOBJ_REGISTER_CLASS_GLOBAL_TEMPLATE(TestSnapObject)
        {
            CppBridge::V8ObjectTemplateBuilder builder(inRuntime->GetIsolate(), inGlobal, "TestSnapObject");
            V8LFuncTpl tpl =
                builder.SetConstuctor(&TestSnapObject::Constructor)
                    .SetProperty("value", &TestSnapObject::GetValue, &TestSnapObject::SetValue)
                    .Build();
            inRuntime->SetObjectTemplate(&TestSnapObject::s_V8CppObjInfo, tpl);
        }

        REGISTER_CLASS_FUNCS_GLOBAL(TestSnapObject);

        TEST_F(JSnapshotCreatorTest, Playground)
        {
            std::filesystem::path snapshotFile = "playground.dat";
            snapshotFile = s_TestDir / snapshotFile;

            JSAppSharedPtr snapApp = m_App->CreateSnapshotApp();
            snapApp->AppInit();
            JSRuntimeSharedPtr runtime = snapApp->GetMainRuntime();

            {
                V8Isolate *isolate = runtime->GetIsolate();
                V8IsolateScope iScope(isolate);
                V8HandleScope hScope(isolate);

                JSContextSharedPtr jsContext = snapApp->CreateJSContext("default", "");
                V8LContext context = jsContext->GetLocalContext();
                V8ContextScope cScope(context);

                const char csource1[] = R"(
                    test('test');
                    test2('test2');
                    var snapObj = new TestSnapObject();
                    snapObj.value = 100;
                )";

                V8LValue result = jsContext->RunScript(csource1);
                if (result.IsEmpty())
                {
                    std::cout << "Script Error: " << JSUtilities::GetStackTrace(context, try_catch) << std::endl;
                    EXPECT_TRUE(false);
                }

                EXPECT_EQ("test", g_FunctionString);
                EXPECT_EQ("test2", g_FunctionString2);
                EXPECT_EQ("test2", g_FunctionString2);
                EXPECT_EQ(100, TestSnapObject::snapObjInstance->GetValue());
            }

            JSSnapshotCreator creator(snapApp);

            EXPECT_TRUE(creator.CreateSnapshotFile(snapshotFile));
            snapApp->DisposeApp();

            V8SnapshotProviderSharedPtr playgroundSnap = std::make_shared<V8SnapshotProvider>(snapshotFile);
            JSAppSharedPtr restore = std::make_shared<JSApp>("Restored", playgroundSnap);
            restore->Initialize(s_TestDir, false, std::make_shared<JSContextCreator>());

            runtime = restore->GetMainRuntime();
            JSContextSharedPtr jsContext = restore->GetMainRuntime()->CreateContext("Restored", "");
            {
                V8Isolate *isolate = runtime->GetIsolate();
                V8IsolateScope iScope(isolate);
                V8HandleScope hScope(isolate);
                V8LContext context = jsContext->GetLocalContext();
                V8ContextScope cScope(context);

                const char csource1[] = R"(
                    test('test3');
                    test2('test4');
                    snapObj.value = 200;
                )";

                V8TryCatch try_catch(isolate);

                V8LString source1 = JSUtilities::StringToV8(isolate, csource1);

                V8LScript script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                V8LValue result;
                if (script1->Run(context).ToLocal(&result) == false)
                {
                    std::cout << "Script Error: " << JSUtilities::GetStackTrace(context, try_catch) << std::endl;
                    EXPECT_TRUE(false);
                }

                EXPECT_EQ("test3", g_FunctionString);
                EXPECT_EQ("test4", g_FunctionString2);
                EXPECT_EQ(200, TestSnapObject::snapObjInstance->GetValue());
            }
            restore->DisposeApp();
        }
    }
}