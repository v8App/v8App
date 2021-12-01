// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>
#include <fstream>

#include "JSUtilites.h"
#include "V8TestFixture.h"
#include "JSUtilites.h"
#include "JSModules.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ScriptStartupDataManager.h"
#include "CppBridge/V8TypeConverter.h"
#include "CppBridge//V8NativeObjectHandle.h"
#include "CppBridge//V8NativeObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSModulesTest = V8TestFixture;

        namespace JSModulesTestInternal
        {
            bool WriteTestFile(std::filesystem::path inFileName, std::string inContents)
            {
                std::ofstream fileStream(inFileName, std::ios::out);
                if (fileStream.is_open() == false)
                {
                    return false;
                }
                fileStream << inContents;
                fileStream.close();
                return true;
            }

            class TestObject *constructerCreatedObjectTest = nullptr;

            class TestObject : public CppBridge::V8NativeObject<TestObject>
            {
            public:
                static CppBridge::V8NativeObjectInfo s_V8NativeObjectInfo;

                static CppBridge::V8NativeObjectHandle<TestObject> CreateObject(v8::Isolate *inIsolate)
                {
                    return CreateV8NativeObjHandle(inIsolate, new TestObject());
                }

                static void BuildObjectTemplate(v8::Isolate *inIsolate)
                {
                    TestObject().GetOrCreateObjectTemplate(inIsolate, &s_V8NativeObjectInfo);
                }

                int GetValue() const { return m_Value; }
                void SetValue(int inValue) { m_Value = inValue; }

                static void Constructor(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
                {
                    v8::Isolate *isolate = inInfo.GetIsolate();

                    if (inInfo.IsConstructCall() == false)
                    {
                        JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::TypeError, "must be an instance call (new)");
                        return;
                    }
                    CppBridge::V8NativeObjectHandle<TestObject> instance = CreateV8NativeObjHandle(isolate, new TestObject());
                    constructerCreatedObjectTest = instance.Get();
                    inInfo.GetReturnValue().Set(CppBridge::ConvertToV8(isolate, instance.Get()).ToLocalChecked());
                }

            protected:
                TestObject() : m_Value(0){};
                virtual ~TestObject() override = default;

                CppBridge::V8ObjectTemplateBuilder GetObjectTemplateBuilder(v8::Isolate *inIsolate) override
                {
                    return V8NativeObject<TestObject>::GetObjectTemplateBuilder(inIsolate)
                        .SetConstuctor("TestObject", &TestObject::Constructor)
                        .SetProperty("value", &TestObject::GetValue, &TestObject::SetValue);
                }

            private:
                int m_Value;

                TestObject(const TestObject &) = delete;
                TestObject &operator=(const TestObject &) = delete;
            };

            CppBridge::V8NativeObjectInfo TestObject::s_V8NativeObjectInfo;

        }

        TEST_F(JSModulesTest, AddGetModulesGetIsolate)
        {
            v8::HandleScope handleScope(m_Isolate);
            JSContext *jsContext = m_Context.lock().get();
            v8::Local<v8::Context> context = jsContext->GetContext();

            //need to create a valid module
            const char source_str[] = R"script(
                let x = 1+1;
            )script";
            v8::Local<v8::String> v8sourceStr = JSUtilities::StringToV8(m_Isolate, source_str);
            v8::ScriptOrigin origin(JSUtilities::StringToV8(m_Isolate, "test"), v8::Local<v8::Integer>(), v8::Local<v8::Integer>(),
                                    v8::Local<v8::Boolean>(), v8::Local<v8::Integer>(), v8::Local<v8::Value>(),
                                    v8::Local<v8::Boolean>(), v8::Local<v8::Boolean>(), v8::True(m_Isolate));

            v8::ScriptCompiler::Source source(v8sourceStr, origin);
            v8::Local<v8::Module> module;
            ASSERT_TRUE(v8::ScriptCompiler::CompileModule(m_Isolate, &source).ToLocal(&module));
            v8::Global<v8::Module> v8Module(m_Isolate, module);
            ASSERT_FALSE(v8Module.IsEmpty());

            //test nullptr for context
            JSModules *jsModule = new JSModules(nullptr, m_Isolate);
            //test get isolate
            EXPECT_EQ(nullptr, jsModule->GetIsolate());

            v8::Local<v8::Module> maybeModule;

            EXPECT_FALSE(jsModule->AddModule(v8Module, "test"));
            EXPECT_EQ(std::string(), jsModule->GetSpecifierByModule(v8Module));
            EXPECT_FALSE(jsModule->GetModuleBySpecifier("test").ToLocal(&maybeModule));

            delete jsModule;
            //test with context now
            jsModule = new JSModules(jsContext, m_Isolate);
            //test get isolate
            EXPECT_EQ(m_Isolate, jsModule->GetIsolate());

            EXPECT_TRUE(jsModule->AddModule(v8Module, "test"));
            EXPECT_EQ("test", jsModule->GetSpecifierByModule(v8Module));
            EXPECT_TRUE(jsModule->GetModuleBySpecifier("test").ToLocal(&maybeModule));
            EXPECT_FALSE(maybeModule.IsEmpty());
            EXPECT_TRUE(v8Module == maybeModule);
        }

        TEST_F(JSModulesTest, SearchPathes)
        {
            //get the path where the test files are we'll use this for the test
            std::string moduleTestDirectory = m_RunFiles->Rlocation("com_github_v8app_v8app/libs/jsRuntime/tests/test-files") + "/modules";
            ASSERT_FALSE(moduleTestDirectory.empty());

            EXPECT_TRUE(JSModules::FindModuleRootPath("search").empty());

            EXPECT_TRUE(JSModules::AddModuleRootPath("search", moduleTestDirectory));
            EXPECT_EQ(moduleTestDirectory, JSModules::FindModuleRootPath("search").string());

            JSModules::RemoveModuleRootPath("search");
            EXPECT_TRUE(JSModules::FindModuleRootPath("search").empty());
        }

        TEST_F(JSModulesTest, LoadRootModule)
        {
            std::string moduleTestDirectory = m_RunFiles->Rlocation("com_github_v8app_v8app/libs/jsRuntime/tests/test-files") + "/modules";

            EXPECT_TRUE(JSModules::AddModuleRootPath("app", moduleTestDirectory));

            JSModules *jsModules = new JSModules(nullptr, m_Isolate);

            //test load root module since a nullptr returns an empty object.
            EXPECT_TRUE(jsModules->LoadModule("test").IsEmpty());

            v8::HandleScope handleScope(m_Isolate);
            JSContext *jsContext = m_Context.lock().get();
            WeakJSModulesPtr weakJSModules = jsContext->GetModules();
            jsModules = weakJSModules.lock().get();

            //test no file found
            v8::Local<v8::Module> root;
            v8::MaybeLocal<v8::Module> maybeRoot = jsModules->LoadModule("noExists");
            EXPECT_FALSE(maybeRoot.ToLocal(&root));
            EXPECT_TRUE(root.IsEmpty());

            //test missing module
            maybeRoot = jsModules->LoadModule("missingModule");
            root.Clear();
            EXPECT_FALSE(maybeRoot.ToLocal(&root));
            EXPECT_TRUE(root.IsEmpty());

            //test no ext
            maybeRoot = jsModules->LoadModule("root");
            root.Clear();
            EXPECT_TRUE(maybeRoot.ToLocal(&root));
            EXPECT_FALSE(root.IsEmpty());

            jsModules->ResetModules();

            //text wrong extension
            maybeRoot = jsModules->LoadModule("root.img");
            root.Clear();
            EXPECT_FALSE(maybeRoot.ToLocal(&root));
            EXPECT_TRUE(root.IsEmpty());

            jsModules->ResetModules();

            //test .mjs
            maybeRoot = jsModules->LoadModule("root.mjs");
            root.Clear();
            EXPECT_TRUE(maybeRoot.ToLocal(&root));
            EXPECT_FALSE(root.IsEmpty());

            jsModules->ResetModules();

            //test .js
            maybeRoot = jsModules->LoadModule("search.js");
            root.Clear();
            EXPECT_TRUE(maybeRoot.ToLocal(&root));
            EXPECT_FALSE(root.IsEmpty());

            jsModules->ResetModules();

            //tets a cahced module by loading a test file that changed after being loaded
            std::filesystem::path testFile = moduleTestDirectory;
            testFile /= "cacheTest.mjs";

            //write a test file that is correct and shuld load
            //first make sure it's not left behind
            std::filesystem::remove(testFile);
            ASSERT_TRUE(JSModulesTestInternal::WriteTestFile(testFile, "import rootImport from 'sub-dir/rootImport';\n let x = 5;"));

            //load it
            maybeRoot = jsModules->LoadModule("cacheTest.mjs");
            root.Clear();
            EXPECT_TRUE(maybeRoot.ToLocal(&root));
            EXPECT_FALSE(root.IsEmpty());

            //write a file that won't compile
            ASSERT_TRUE(JSModulesTestInternal::WriteTestFile(testFile, "import rootImport from 'sub-dir/rootImport;'\n let x = ;"));

            //it should still load since it now comes form the module cache
            maybeRoot = jsModules->LoadModule("cacheTest.mjs");
            root.Clear();
            EXPECT_TRUE(maybeRoot.ToLocal(&root));
            EXPECT_FALSE(root.IsEmpty());

            //just to make sure clear the cache and try again
            jsModules->ResetModules();
            //Clear the script cache
            ScriptStartupDataManager::ClearCache();

            maybeRoot = jsModules->LoadModule("cacheTest.mjs");
            root.Clear();
            EXPECT_FALSE(maybeRoot.ToLocal(&root));
            EXPECT_TRUE(root.IsEmpty());

            std::filesystem::remove(testFile);
        }

        v8::MaybeLocal<v8::Module> LoadModuleTree(JSContext *inContext, const std::filesystem::path &inFileName)
        {
            std::filesystem::path filePath = inFileName;
            v8::Isolate *isolate = inContext->GetIsolate();
            //try to read the file as is
            filePath = JSModules::FindModuleRootPath(filePath);
            //if empty mvoe to .js
            if (filePath.empty())
            {
                return v8::MaybeLocal<v8::Module>();
            }

            std::string sourceText;
            if (ScriptStartupDataManager::LoadScriptFile(filePath, sourceText) == false)
            {
                return v8::MaybeLocal<v8::Module>();
            }

            v8::Local<v8::String> v8SourceText = JSUtilities::StringToV8(isolate, sourceText);
            if (v8SourceText.IsEmpty())
            {
                return v8::MaybeLocal<v8::Module>();
            }

            v8::ScriptOrigin origin(JSUtilities::StringToV8(isolate, inFileName), v8::Local<v8::Integer>(), v8::Local<v8::Integer>(),
                                    v8::Local<v8::Boolean>(), v8::Local<v8::Integer>(), v8::Local<v8::Value>(),
                                    v8::Local<v8::Boolean>(), v8::Local<v8::Boolean>(), v8::True(isolate));
            v8::ScriptCompiler::Source source(v8SourceText, origin);
            v8::Local<v8::Module> module;
            if (v8::ScriptCompiler::CompileModule(isolate, &source).ToLocal(&module) == false)
            {
                return v8::MaybeLocal<v8::Module>();
            }

            std::weak_ptr<JSModules> weakModules = inContext->GetModules();
            JSModules *modules = weakModules.lock().get();
            //add the test module to the loaded modules
            modules->AddModule(v8::Global<v8::Module>(isolate, module), inFileName);

            std::filesystem::path moduleDir = inFileName;
            moduleDir.remove_filename();

            //for testing purposes we don't load the rest of it's requested imports
            return module;
        }

        v8::MaybeLocal<v8::Module> LoadModuleForTest(JSContext *inContext, std::string inModuleName)
        {
            if (inContext == nullptr)
            {
                return v8::MaybeLocal<v8::Module>();
            }

            v8::Isolate *isolate = inContext->GetIsolate();
            v8::EscapableHandleScope handleScope(isolate);
            v8::Context::Scope contextScope(inContext->GetContext());
            v8::TryCatch tryCatch(isolate);

            v8::Local<v8::Module> rootModule;

            if (LoadModuleTree(inContext, inModuleName).ToLocal(&rootModule) == false)
            {
                return v8::MaybeLocal<v8::Module>();
            }

            return handleScope.Escape(rootModule);
        }

        TEST_F(JSModulesTest, InstantiateModule)
        {
            std::string moduleTestDirectory = m_RunFiles->Rlocation("com_github_v8app_v8app/libs/jsRuntime/tests/test-files") + "/modules";

            EXPECT_TRUE(JSModules::AddModuleRootPath("app", moduleTestDirectory));

            v8::HandleScope handleScope(m_Isolate);
            JSContext *jsContext = m_Context.lock().get();
            WeakJSModulesPtr weakJSModules = jsContext->GetModules();
            JSModules *jsModules = weakJSModules.lock().get();

            //test that module can't be instantiate cause imported module isn't loaded
            v8::Local<v8::Module> root;
            v8::MaybeLocal<v8::Module> maybeRoot = LoadModuleForTest(jsContext, "root.mjs");
            EXPECT_FALSE(maybeRoot.ToLocal(&root));
            ASSERT_TRUE(root.IsEmpty());

            EXPECT_FALSE(jsModules->InstantiateModule(root));

            //reset the modules so that we can reload the root module
            jsModules->ResetModules();

            maybeRoot = jsModules->LoadModule("root.mjs");
            EXPECT_TRUE(maybeRoot.ToLocal(&root));
            EXPECT_FALSE(root.IsEmpty());

            EXPECT_TRUE(jsModules->InstantiateModule(root));
        }

        TEST_F(JSModulesTest, RunModule)
        {
            std::string moduleTestDirectory = m_RunFiles->Rlocation("com_github_v8app_v8app/libs/jsRuntime/tests/test-files") + "/modules";

            EXPECT_TRUE(JSModules::AddModuleRootPath("app", moduleTestDirectory));

            v8::HandleScope handleScope(m_Isolate);
            JSContext *jsContext = m_Context.lock().get();
            WeakJSModulesPtr weakJSModules = jsContext->GetModules();
            JSModules *jsModules = weakJSModules.lock().get();

            JSModulesTestInternal::TestObject::BuildObjectTemplate(m_Isolate);

            const char source[] = R"script(
                        let obj = new TestObject();
                )script";
            v8::Local<v8::String> v8Source = JSUtilities::StringToV8(m_Isolate, source);
            EXPECT_FALSE(v8Source.IsEmpty());

            v8::TryCatch tryCatch(m_Isolate);

            v8::Local<v8::Script> script = v8::Script::Compile(jsContext->GetContext(), v8Source).ToLocalChecked();
            script->Run(jsContext->GetContext());
            if (tryCatch.HasCaught())
            {
                std::string error = JSUtilities::GetStackTrace(m_Isolate, tryCatch);
                std::cout << "Script Error: " << error << std::endl;
                ASSERT_TRUE(false);
            }

            EXPECT_NE(nullptr, JSModulesTestInternal::constructerCreatedObjectTest);
            JSModulesTestInternal::constructerCreatedObjectTest) = nullptr;

            //test that module can't be instantiate cause imported module isn't loaded
            v8::Local<v8::Module> root;
            v8::MaybeLocal<v8::Module> maybeRoot = jsModules->LoadModule("root.mjs");
            EXPECT_TRUE(maybeRoot.ToLocal(&root));
            ASSERT_FALSE(root.IsEmpty());

            EXPECT_TRUE(jsModules->InstantiateModule(root));

            EXPECT_TRUE(jsModules->RunModule(root));

            ASSERT_NE(nullptr, JSModulesTestInternal::constructerCreatedObjectTest);

            EXPECT_EQ(5, JSModulesTestInternal::constructerCreatedObjectTest->GetValue());
        }
    }
}