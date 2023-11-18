// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <algorithm>
#include <format>
#include <regex>

#include "Assets/AppAssetRoots.h"
#include "JSContext.h"
#include "JSContextModules.h"
#include "JSUtilities.h"
#include "Logging/LogMacros.h"
#include "ScriptStartupDataManager.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace internal
        {
            struct ModuleCallbackData
            {
                JSContextSharedPtr m_Context;
                v8::Global<v8::Value> m_ResourceName;
                v8::Global<v8::String> m_Specifier;
                v8::Global<v8::Promise::Resolver> m_Resolver;
                v8::Global<v8::FixedArray> m_ImportAssertions;

                ModuleCallbackData(JSContextSharedPtr inContext, v8::Local<v8::Value> &inResourcecName, v8::Local<v8::String> &inSpecifier,
                                   v8::Local<v8::Promise::Resolver> &inResolver, v8::Local<v8::FixedArray> &inImportAssertions)
                {
                    v8::Isolate *isolate = inContext->GetIsolate();
                    m_Context = inContext;
                    m_ResourceName.Reset(isolate, inResourcecName);
                    m_Specifier.Reset(isolate, inSpecifier);
                    m_Resolver.Reset(isolate, inResolver);
                    m_ImportAssertions.Reset(isolate, inImportAssertions);
                }
            };
        } // internal

        JSContextModules::JSContextModules(JSContextSharedPtr inContext) : m_Context(inContext),
                                                                           m_ModuleToSpecifierMap(10, ModuleHash(inContext)),
                                                                           m_JSONModuleToParsedMap(10, ModuleHash(inContext))
        {
        }

        JSContextModules::~JSContextModules()
        {
            m_Context = nullptr;
        }

        v8::Isolate *JSContextModules::GetIsolate() const
        {
            return m_Context == nullptr ? nullptr : m_Context->GetIsolate();
        }

        bool JSContextModules::AddModule(const JSModuleInfoSharedPtr &inModule, std::string inFileName, JSModuleInfo::ModuleType inModuleType)
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return false;
            }
            if (m_ModuleToSpecifierMap.insert(std::make_pair(inModule, inFileName)).second == false)
            {
                return false;
            }
            if (m_ModuleMap.insert(std::make_pair(std::make_pair(inFileName, inModuleType), inModule)).second == false)
            {
                return false;
            }
            return true;
        }

        bool JSContextModules::AddJSONModule(const JSModuleInfoSharedPtr &inModule, v8::Global<v8::Value> &inParsedJSON, std::string inFileName, JSModuleInfo::ModuleType inModuleType)
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return false;
            }
            if (AddModule(inModule, inFileName, inModuleType) == false)
            {
                return false;
            }
//            if (m_JSONModuleToParsedMap.insert(std::make_pair(inModule, inParsedJSON)).second == false)
            {
                return false;
            }
        }

        JSModuleInfoSharedPtr JSContextModules::GetModuleBySpecifier(std::string inSpecifier)
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return JSModuleInfoSharedPtr();
            }
            auto it = std::find_if(std::begin(m_ModuleToSpecifierMap), std::end(m_ModuleToSpecifierMap),
                                   [inSpecifier](auto &&p)
                                   { return p.second == inSpecifier; });
            if (it == m_ModuleToSpecifierMap.end())
            {
                return JSModuleInfoSharedPtr();
            }
            return it->first;
        }

        std::string JSContextModules::GetSpecifierByModule(const JSModuleInfoSharedPtr &inModule)
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return std::string();
            }
            auto it = m_ModuleToSpecifierMap.find(inModule);
            if (it == m_ModuleToSpecifierMap.end())
            {
                return std::string();
            }
            return it->second;
        }

        v8::Local<v8::Value> JSContextModules::GetJSONByModule(const JSModuleInfoSharedPtr &inModule)
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return v8::Local<v8::Value>();
            }
            auto it = m_JSONModuleToParsedMap.find(inModule);
            if (it == m_JSONModuleToParsedMap.end())
            {
                return v8::Local<v8::Value>();
            }
            return it->second.Get(isolate);
        }

        void JSContextModules::ResetModules()
        {
            m_ModuleMap.clear();
            m_ModuleToSpecifierMap.clear();
            m_JSONModuleToParsedMap.clear();
        }

        void JSContextModules::SetupModulesCallbacks(v8::Isolate *inIsolate)
        {
            inIsolate->SetHostImportModuleDynamicallyCallback(JSContextModules::HostImportModuleDynamically);
            inIsolate->SetHostInitializeImportMetaObjectCallback(JSContextModules::InitializeImportMetaObject);
        }

        JSModuleInfo::AssertionInfo JSContextModules::GetModuleAssertionInfo(v8::Local<v8::Context> inContext, v8::Local<v8::FixedArray> inAssertions, bool inHasPostions)
        {
            v8::Isolate *isolate = inContext->GetIsolate();
            const int kEntrySize = inHasPostions ? 3 : 2;
            JSModuleInfo::AssertionInfo returnInfo;
            returnInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);

            if (isolate == nullptr || jsContext == nullptr)
            {
                // TODO:: Log message
                returnInfo.m_Type = JSModuleInfo::ModuleType::kInvalid;
                return returnInfo;
            }

            for (int i = 0; i < inAssertions->Length(); i += kEntrySize)
            {
                v8::Local<v8::String> v8Key = inAssertions->Get(inContext, i).As<v8::String>();
                std::string key = JSUtilities::V8ToString(isolate, v8Key);
                if (key == "type" || key == "version" || key == "module")
                {
                    v8::Local<v8::String> v8Value = inAssertions->Get(inContext, i + 1).As<v8::String>();
                    std::string value = JSUtilities::V8ToString(isolate, v8Value);
                    if (key == "type")
                    {
                        returnInfo.m_TypeString = value;
                        if (value == "json")
                        {
                            returnInfo.m_Type = JSModuleInfo::ModuleType::kJSON;
                        }
                        else if (value == "js")
                        {
                            returnInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
                        }
                        else if (value == "native")
                        {
                            returnInfo.m_Type = JSModuleInfo::ModuleType::kNative;
                        }
                        else
                        {
                            returnInfo.m_Type = JSModuleInfo::ModuleType::kInvalid;
                        }
                    }
                    if (key == "version")
                    {
                        Utils::VersionString version = Utils::VersionString(value);
                        if (version.IsVersionString() == false)
                        {
                            returnInfo.m_Type = JSModuleInfo::ModuleType::kInvalid;
                            return returnInfo;
                        }
                        returnInfo.m_Version = version;
                    }
                    if (key == "module")
                    {
                        returnInfo.m_Module = value;
                    }
                }
            }
            // version has to be specified with module or it's ignored
            if (returnInfo.m_Version.IsVersionString() && returnInfo.m_Module.empty())
            {
                returnInfo.m_Version = Utils::VersionString();
            }

            return returnInfo;
        }

        bool JSContextModules::BuildModulePath(const JSModuleInfo::AssertionInfo &inAssertionInfo, std::filesystem::path &inImportPath)
        {
            std::filesystem::path modulePath;
            auto it = inImportPath.begin();
            bool moduleRoot = false;
            Assets::AppAssetRootsSharedPtr appRoots = m_Context->GetJSRuntime()->GetAppRoots();

            std::string module = (*it).string();
            Utils::VersionString version = Utils::VersionString();

            std::filesystem::path mocdulePath;

            // all registered modules should have a latest version except the js and resource root
            if (module != Assets::c_RootJS && module != Assets::c_RootResource)
            {
                moduleRoot = true;
                // if it's the modules directory then discard it so we get the module name
                if (module == Assets::c_RootModules)
                {
                    it++;
                    module = (*it).string();
                }
                if (appRoots->GetModulesLatestVersion(module).IsVersionString())
                {
                    modulePath = module;
                    ++it;
                    version = Utils::VersionString((*it).string());
                    if (version.IsVersionString())
                    {
                        modulePath /= version.GetVersionString();
                    }
                    else
                    {
                        // module version string is either an invalid version or not one at all
                        // default to the latest
                        modulePath /= appRoots->GetModulesLatestVersion(module).GetVersionString();
                    }
                    // ok we have the module's name for it's root
                    if (appRoots->FindModuleRootPath(modulePath.string()).empty())
                    {
                        // module/version doesn't exist
                        return false;
                    }
                }
            }
            else
            {
                modulePath = appRoots->FindModuleRootPath(module) / inImportPath;
            }

            if (inAssertionInfo.m_Module.length())
            {
                std::filesystem::path assertedModule = inAssertionInfo.m_Module;
                if (inAssertionInfo.m_Version.IsVersionString())
                {
                    assertedModule /= std::filesystem::path(inAssertionInfo.m_Version.GetVersionString());
                }
                else
                {
                    assertedModule /= std::filesystem::path(appRoots->GetModulesLatestVersion(inAssertionInfo.m_Module).GetVersionString());
                }
                // if we have a module root then verify it against the assertions
                if (moduleRoot && assertedModule != modulePath)
                {
                    return false;
                }
                else
                {
                    // we don't have  amodule root and have a module assertion so add the module path
                    modulePath = appRoots->FindModuleRootPath(assertedModule.string()) / inImportPath;
                }
            }
            inImportPath = modulePath;
            return true;
        }

        v8::MaybeLocal<v8::Module> JSContextModules::LoadEntryPoint(std::filesystem::path inMainPath)
        {
            if (m_Context == nullptr)
            {
                return v8::MaybeLocal<v8::Module>();
            }

            Assets::AppAssetRootsSharedPtr appRoot = m_Context->GetJSRuntime()->GetAppRoots();

            v8::Isolate *isolate = m_Context->GetIsolate();
            if (isolate == nullptr)
            {
                return v8::MaybeLocal<v8::Module>();
            }
            v8::EscapableHandleScope handleScope(isolate);
            v8::Context::Scope contextScope(m_Context->GetContext());
            v8::TryCatch tryCatch(isolate);

            v8::Local<v8::Module> rootModule;
            std::filesystem::path rootPath;
            JSModuleInfo::AssertionInfo info;
            info.m_Type = JSModuleInfo::ModuleType::kJavascript;

            if (LoadModuleTree(m_Context, inMainPath, appRoot->GetAppRoot(), info).ToLocal(&rootModule) == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Failed to load main module");
                msg.emplace(Log::MsgKey::File, inMainPath.string());
                if (tryCatch.HasCaught())
                {
                    msg.emplace(Log::MsgKey::StackTrace, JSUtilities::GetStackTrace(isolate, tryCatch));
                }
                LOG_ERROR(msg);
                return v8::MaybeLocal<v8::Module>();
            }

            return handleScope.Escape(rootModule);
        }

        bool JSContextModules::InstantiateModule(v8::Local<v8::Module> inModule)
        {
            if (inModule.IsEmpty())
            {
                return false;
            }

            return inModule->InstantiateModule(m_Context->GetContext(), ResolveModuleCallback).FromMaybe(false);
        }

        bool JSContextModules::RunModule(v8::Local<v8::Module> inModule)
        {
            if (inModule.IsEmpty())
            {
                return false;
            }
            v8::TryCatch tryCatch(m_Context->GetIsolate());

            inModule->Evaluate(m_Context->GetContext());
            if (tryCatch.HasCaught())
            {
                Log::LogMessage message = {
                    {Log::MsgKey::Msg, "Failed to run the module"},
                    {Log::MsgKey::StackTrace, JSUtilities::GetStackTrace(m_Context->GetIsolate(), tryCatch)}};
                LOG_ERROR(message);
                return false;
            }
            return false;
        }

        v8::MaybeLocal<v8::Promise> JSContextModules::HostImportModuleDynamically(v8::Local<v8::Context> inContext,
                                                                                  v8::Local<v8::Data> inDefinedOptions,
                                                                                  v8::Local<v8::Value> inResourceName,
                                                                                  v8::Local<v8::String> inSpecifier,
                                                                                  v8::Local<v8::FixedArray> importAssertions)
        {
            // for now we don't have an defined options
            v8::Isolate *isolate = inContext->GetIsolate();
            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);

            v8::MaybeLocal<v8::Promise::Resolver> maybeResolver = v8::Promise::Resolver::New(inContext);
            v8::Local<v8::Promise::Resolver> resolver;

            if (maybeResolver.ToLocal(&resolver) == false)
            {
                return v8::MaybeLocal<v8::Promise>();
            }

            if (inResourceName->IsNull())
            {
                resolver->Reject(inContext, v8::Exception::TypeError(v8::String::NewFromUtf8Literal(isolate, "Resource name is empty"))).ToChecked();
            }
            else
            {
                internal::ModuleCallbackData *data = new internal::ModuleCallbackData(jsContext, inResourceName,
                                                                                      inSpecifier, resolver, importAssertions);
                isolate->EnqueueMicrotask(JSContextModules::ImportModuleDynamicallyMicroTask, data);
            }
            return resolver->GetPromise();
        }

        void JSContextModules::ImportModuleDynamicallyMicroTask(void *inData)
        {
            std::unique_ptr<internal::ModuleCallbackData> importData(static_cast<internal::ModuleCallbackData *>(inData));
            v8::Isolate *isolate = importData->m_Context->GetIsolate();
            if (isolate == nullptr)
            {
                // need to fatal error here
                return;
            }

            JSContextModulesSharedPtr jsModules = importData->m_Context->GetJSModules();

            v8::HandleScope handleScope(isolate);
            v8::Local<v8::Context> context = importData->m_Context->GetContext();

            v8::Local<v8::Value> referrer = importData->m_ResourceName.Get(isolate);
            v8::Local<v8::String> specifier = importData->m_Specifier.Get(isolate);
            v8::Local<v8::Promise::Resolver> resolver = importData->m_Resolver.Get(isolate);
            v8::Local<v8::FixedArray> importAssertions = importData->m_ImportAssertions.Get(isolate);

            v8::Context::Scope contextScope(context);

            JSModuleInfo::AssertionInfo assertionInfo = jsModules->GetModuleAssertionInfo(context, importAssertions, false);

            v8::TryCatch tryCatch(isolate);

            if (assertionInfo.m_Type == JSModuleInfo::ModuleType::kInvalid)
            {
                std::string specifierStr = JSUtilities::V8ToString(isolate, specifier);
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, std::format("Import '{}' had an invalid type of '{}'", specifierStr, assertionInfo.m_TypeString));
                resolver->Reject(context, tryCatch.Exception()).ToChecked();
                return;
            }

            std::filesystem::path modulePath = JSUtilities::V8ToString(isolate, referrer);
            if (modulePath.empty())
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Import path was empty.");
                resolver->Reject(context, tryCatch.Exception()).ToChecked();
                return;
            }

            tryCatch.SetVerbose(true);
            v8::Local<v8::Module> module;
            if (LoadModuleTree(importData->m_Context, modulePath, "", assertionInfo).ToLocal(&module) == false)
            {
                CHECK(tryCatch.HasCaught());
                resolver->Reject(context, tryCatch.Exception()).ToChecked();
                return;
            }

            v8::MaybeLocal<v8::Value> maybeResult;
            if (module->InstantiateModule(context, JSContextModules::ResolveModuleCallback).FromMaybe(false) == false)
            {
                CHECK(tryCatch.HasCaught());
                resolver->Reject(context, tryCatch.Exception()).ToChecked();
                return;
            }

            maybeResult = module->Evaluate(importData->m_Context->GetContext());
            if (maybeResult.IsEmpty() == false)
            {
                v8::MicrotasksScope::PerformCheckpoint(isolate);
            }

            v8::Local<v8::Value> result;
            if (maybeResult.ToLocal(&result) == false)
            {
                DCHECK_TRUE(tryCatch.HasCaught());
                resolver->Reject(context, tryCatch.Exception()).ToChecked();
                return;
            }

            v8::Local<v8::Value> moduleNamespace = module->GetModuleNamespace();
            if (result->IsPromise() == false)
            {
                if (tryCatch.HasCaught())
                {
                    resolver->Reject(context, JSUtilities::StringToV8(isolate, "Expected promise to be returned from evaluated module "));
                }
                else
                {
                    resolver->Resolve(context, moduleNamespace).ToChecked();
                }
                return;
            }
            v8::Local<v8::Promise> resultPromise(v8::Local<v8::Promise>::Cast(result));
            if (resultPromise->State() == v8::Promise::kRejected)
            {
                resolver->Reject(context, resultPromise->Result()).ToChecked();
                return;
            }
            v8::Local<v8::Value> DummyValue;
            v8::Local<v8::String> DummyString;
            v8::Local<v8::FixedArray> DummyArray;
            internal::ModuleCallbackData *data = new internal::ModuleCallbackData(importData->m_Context, DummyValue, DummyString, resolver, DummyArray);
            v8::Local<v8::External> eData = v8::External::New(isolate, data);
            v8::Local<v8::Function> callbackResolve;
            if (v8::Function::New(context, JSContextModules::ResolvePromiseCallback, eData).ToLocal(&callbackResolve) == false)
            {
                resolver->Reject(context, JSUtilities::StringToV8(isolate, "Failed to create the module resolver resolve callback function")).ToChecked();
                return;
            }
            v8::Local<v8::Function> callbackReject;
            if (v8::Function::New(context, JSContextModules::RejectPromiseCallback, eData).ToLocal(&callbackReject) == false)
            {
                resolver->Reject(context, JSUtilities::StringToV8(isolate, "Failed to create the module resolver reject callback function")).ToChecked();
                return;
            }
            resultPromise->Then(context, callbackResolve, callbackReject).ToLocalChecked();
        }

        void JSContextModules::InitializeImportMetaObject(v8::Local<v8::Context> inContext, v8::Local<v8::Module> inModule, v8::Local<v8::Object> inMeta)
        {
            v8::Isolate *isolate = inContext->GetIsolate();
            v8::HandleScope handleScope(isolate);

            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);
            CHECK_NOT_NULL(jsContext);
            JSContextModulesSharedPtr JSContextModules = jsContext->GetJSModules();
            CHECK_NOT_NULL(JSContextModules);

            /*
            std::string specifier = JSContextModules->GetSpecifierByModule(v8::Global<v8::Module>(isolate, inModule));
            if (specifier.length() == 0)
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "URL specifier for module was empty");
                return;
            }

            v8::Local<v8::String> urlKey = JSUtilities::StringToV8(isolate, "url", v8::NewStringType::kInternalized);
            v8::Local<v8::String> url = JSUtilities::StringToV8(isolate, specifier);
            if (url->Length() == 0)
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "V8 URL specifier for module was empty");
                return;
            }

            if (inMeta->CreateDataProperty(inContext, urlKey, url).ToChecked() == false)
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::ReferenceError, "Failed to create the urlKey for the module's meta");
            }
            */
        }

        void JSContextModules::ResolvePromiseCallback(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
        {
            std::unique_ptr<internal::ModuleCallbackData> importData(static_cast<internal::ModuleCallbackData *>(inInfo.Data().As<v8::External>()->Value()));
            v8::Isolate *isolate = importData->m_Context->GetIsolate();
            v8::Local<v8::Context> context = importData->m_Context->GetContext();

            v8::HandleScope handleScope(isolate);
            /*
                        v8::Local<v8::Promise::Resolver> resolver(importData->m_Resolver.Get(isolate));
                        v8::Local<v8::Value> moduleNamespace = importData->m_Namespace.Get(isolate);
                        v8::Context::Scope contextScope(context);

                        resolver->Resolve(context, moduleNamespace).ToChecked();
                        */
        }

        void JSContextModules::RejectPromiseCallback(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
        {
            std::unique_ptr<internal::ModuleCallbackData> importData(static_cast<internal::ModuleCallbackData *>(inInfo.Data().As<v8::External>()->Value()));
            v8::Isolate *isolate = importData->m_Context->GetIsolate();
            v8::Local<v8::Context> context = importData->m_Context->GetContext();

            v8::HandleScope handleScope(isolate);
            v8::Local<v8::Promise::Resolver> resolver(importData->m_Resolver.Get(isolate));
            v8::Context::Scope contextScope(context);

            DCHECK_EQ(inInfo.Length(), 1);
            resolver->Reject(context, inInfo[0]).ToChecked();
        }

        v8::MaybeLocal<v8::Module> JSContextModules::ResolveModuleCallback(v8::Local<v8::Context> inContext, v8::Local<v8::String> inSpecifier, v8::Local<v8::FixedArray> inAssertions, v8::Local<v8::Module> inReferrer)
        {
            v8::Isolate *isolate = inContext->GetIsolate();
            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);

            JSContextModulesSharedPtr jsModule = jsContext->GetJSModules();
            /*
                        std::filesystem::path specifier = jsModule->GetSpecifierByModule(v8::Global<v8::Module>(isolate, inReferrer));
                        if (specifier == "")
                        {
                            JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Failed to ind the module");
                            return v8::MaybeLocal<v8::Module>();
                        }
                        specifier.remove_filename();
                        specifier /= JSUtilities::V8ToString(isolate, inSpecifier);

                        //            v8::MaybeLocal<v8::Module> module = jsModule->GetModuleBySpecifier(specifier);
                        if (module.IsEmpty())
                        {
                            JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Failed to ind the module for " + specifier.filename().string());
                            return v8::MaybeLocal<v8::Module>();
                        }

                        return module;
                        */
                       return v8::MaybeLocal<v8::Module>();
        }

        v8::MaybeLocal<v8::Module> JSContextModules::LoadModuleTree(JSContextSharedPtr inContext, const std::filesystem::path &inImportPath, std::filesystem::path inModuleRoot, const JSModuleInfo::AssertionInfo &inAssertionInfo)
        {
            v8::Isolate *isolate = inContext->GetIsolate();
            Assets::AppAssetRootsSharedPtr appRoot = inContext->GetJSRuntime()->GetAppRoots();
            JSContextModulesSharedPtr jsModule = inContext->GetJSModules();
            std::string moduleName;
            std::filesystem::path importPath = appRoot->MakeRelativePathToRoot(inImportPath, inModuleRoot);
            // if an absolute path then make sure it's rooted to the app root and then make it relative
            if (importPath.empty())
            {
                return v8::MaybeLocal<v8::Module>();
            }

            // file can only be located in the root of the app or the js folder
            auto it = importPath.begin();
            if (((*it).string() == Assets::c_RootModules || (*it).string() == Assets::c_RootResource || importPath.filename() != importPath) && (*it).string() != Assets::c_RootJS)
            {
                return v8::MaybeLocal<v8::Module>();
            }

            if (jsModule->BuildModulePath(inAssertionInfo, importPath) == false)
            {
                return v8::MaybeLocal<v8::Module>();
            }

            if (inModuleRoot.empty())
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Failed to find the module file: " + inImportPath.string());
                return v8::MaybeLocal<v8::Module>();
            }

            JSContextModulesSharedPtr modules = inContext->GetJSModules();
            /*
                        std::string sourceText;
                        if (ScriptStartupDataManager::LoadScriptFile(importPath, sourceText) == false)
                        {
                            JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Failed to load the module file: " + inImportPath);
                            return v8::MaybeLocal<v8::Module>();
                        }

                        v8::Local<v8::String> v8SourceText = JSUtilities::StringToV8(isolate, sourceText);
                        if (v8SourceText.IsEmpty())
                        {
                            JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Failed to fine the module file: " + inImportPath);
                            return v8::MaybeLocal<v8::Module>();
                        }

                        v8::ScriptOrigin origin(JSUtilities::StringToV8(isolate, importPath.string()), v8::Local<v8::Integer>(), v8::Local<v8::Integer>(),
                                                v8::Local<v8::Boolean>(), v8::Local<v8::Integer>(), v8::Local<v8::Value>(),
                                                v8::Local<v8::Boolean>(), v8::Local<v8::Boolean>(), v8::True(isolate));
                        v8::ScriptCompiler::Source source(v8SourceText, origin);
                        v8::Local<v8::Module> module;
                        if (v8::ScriptCompiler::CompileModule(isolate, &source).ToLocal(&module) == false)
                        {
                            return v8::MaybeLocal<v8::Module>();
                        }

                        modules->AddModule(v8::Global<v8::Module>(isolate, module), importPath);

                        for (int idx = 0; idx < module->GetModuleRequestsLength(); idx++)
                        {
                            importPathStr = JSUtilities::V8ToString(isolate, module->GetModuleRequest(idx));
                            // Load module tree will determine after parsing the import if the module is already loaded.
                            if (LoadModuleTree(inContext, importPathStr, inModuleRoot).IsEmpty())
                            {
                                return v8::MaybeLocal<v8::Module>();
                            }
                        }
                        return module;
                        */
        }
        size_t JSContextModules::ModuleHash::operator()(const JSModuleInfoSharedPtr &inModule) const
        {
            if (inModule == nullptr)
            {
                return -1;
            }
            return inModule->GetV8Module(m_Context->GetIsolate())->GetIdentityHash();
        }
    } // JSRuntime
} // JSApp