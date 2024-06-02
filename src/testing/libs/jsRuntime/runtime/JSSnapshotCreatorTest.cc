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
#include "CppBridge/V8NativeObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"

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

        void RegisterFuncTemplate(JSRuntimeSharedPtr inRuntime, v8::Local<v8::ObjectTemplate> &inGlobal)
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

        class TestSnapObject : public CppBridge::V8NativeObject<TestSnapObject>
        {
        public:
            static inline TestSnapObject *snapObjInstance = nullptr;

            DEF_V8NATIVE_FUNCTIONS(TestSnapObject);

            static V8LocalObject Constructor(V8Isolate *isolate)
            {
                snapObjInstance = new TestSnapObject();
                return snapObjInstance->GetV8NativeObjectInternal(isolate, &TestSnapObject::s_V8NativeObjectInfo).ToLocalChecked();
            }

            void SetValue(int inValue) { m_Value = inValue; }
            int GetValue() { return m_Value; }

        protected:
            int m_Value = 5;
            TestSnapObject() {}
            virtual ~TestSnapObject() = default;
        };

        IMPL_DESERIALIZER(TestSnapObject)
        {
            TestSnapObject::snapObjInstance = new TestSnapObject();
            int *value = (int *)inSerialized.data;
            TestSnapObject::snapObjInstance->m_Value = *value;
            return TestSnapObject::snapObjInstance;
        }

        IMPL_SERIALIZER(TestSnapObject)
        {
            TestSnapObject *instance = reinterpret_cast<TestSnapObject *>(inNativeObject);
            v8::StartupData data;
            int *value = new int(instance->m_Value);
            data.data = (const char *)value;
            data.raw_size = sizeof(int);
            return data;
        }

        IMPL_REGISTER_CLASS_FUNCS(TestSnapObject)
        {
            CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestSnapObject::Constructor));
            CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestSnapObject::SetValue));
            CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestSnapObject::GetValue));
        }

        IMPL_REGISTER_CLASS_GLOBAL_TEMPLATE(TestSnapObject)
        {
            CppBridge::V8ObjectTemplateBuilder builder(inRuntime->GetIsolate(), inGlobal, "TestSnapObject");
            v8::Local<v8::ObjectTemplate> tpl =
                builder.SetConstuctor(&TestSnapObject::Constructor)
                    .SetProperty("value", &TestSnapObject::GetValue, &TestSnapObject::SetValue)
                    .Build();
            inRuntime->SetObjectTemplate(&TestSnapObject::s_V8NativeObjectInfo, tpl);
        }

        REGISTER_CLASS_FUNCS(TestSnapObject);

        TEST_F(JSnapshotCreatorTest, Playground)
        {
            std::filesystem::path snapshotFile = "playground.dat";
            snapshotFile = s_TestDir / snapshotFile;

            m_App->SetEntryPointScript("%JS%/LoadModules.js");

            JSAppSharedPtr snapApp = m_App->CreateSnapshotApp();
            snapApp->AppInit();
            JSRuntimeSharedPtr runtime = snapApp->GetJSRuntime();
            runtime->CreateGlobalTemplate(true);

            {
                v8::Isolate *isolate = snapApp->GetJSRuntime()->GetIsolate();
                v8::Isolate::Scope iScope(isolate);
                v8::HandleScope hScope(isolate);

                JSContextSharedPtr jsContext = snapApp->CreateJSContext("default");
                V8LocalContext context = jsContext->GetLocalContext();
                v8::Context::Scope cScope(context);

                const char csource1[] = R"(
                    test('test');
                    test2('test2');
                    let snapObj = new TestSnapObject();
                    snapObj.value = 100;
                )";

                v8::TryCatch try_catch(isolate);

                v8::Local<v8::String> source1 = JSUtilities::StringToV8(isolate, csource1);

                v8::Local<v8::Script> script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                v8::Local<v8::Value> result;
                if (script1->Run(context).ToLocal(&result) == false)
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

            JSSnapshotProviderSharedPtr playgroundSnap = std::make_shared<JSSnapshotProvider>(snapshotFile);
            JSAppSharedPtr restore = std::make_shared<JSApp>("Restored", playgroundSnap);
            restore->Initialize(s_TestDir, false, std::make_shared<JSContextCreator>());

            runtime = restore->GetJSRuntime();
            runtime->CreateGlobalTemplate(false);
            JSContextSharedPtr jsContext = restore->CreateJSContext("Restored");
            {
                v8::Isolate *isolate = runtime->GetIsolate();
                v8::Isolate::Scope iScope(isolate);
                v8::HandleScope hScope(isolate);
                V8LocalContext context = jsContext->GetLocalContext();
                v8::Context::Scope cScope(context);

                const char csource1[] = R"(
                    test('test3');
                    test2('test4');
                    snapObj.value = 200;
                )";

                v8::TryCatch try_catch(isolate);

                v8::Local<v8::String> source1 = JSUtilities::StringToV8(isolate, csource1);

                v8::Local<v8::Script> script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                v8::Local<v8::Value> result;
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