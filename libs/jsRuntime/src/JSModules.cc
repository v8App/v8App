// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "JSModules.h"
#include "JSContext.h"
#include "JSUtilites.h"
#include "Logging/LogMacros.h"
#include "ScriptStartupDataManager.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace internal
        {
            ModuleCallbackData::ModuleCallbackData(JSContext *inContext, v8::Local<v8::String> inReferrer, v8::Local<v8::String> inSpecifier,
                                                   v8::Local<v8::Promise::Resolver> inResolver, v8::Local<v8::Value> inNamespace) : m_Context(inContext)
            {
                v8::Isolate *isolate = inContext->GetIsolate();
                m_Specifier.Reset(isolate, inSpecifier);
                m_Referrer.Reset(isolate, inReferrer);
                m_Resolver.Reset(isolate, inResolver);
                m_Namespace.Reset(isolate, inNamespace);
            }
        }

        std::map<std::string, std::filesystem::path> JSModules::s_ModuleRoots;

        JSModules::~JSModules()
        {
            m_Context = nullptr;
        }

        v8::Isolate *JSModules::GetIsolate() const
        {
            if (m_Context)
            {
                return m_Context->GetIsolate();
            }
            return nullptr;
        }

        bool JSModules::AddModule(const v8::Global<v8::Module> &inModule, std::string inReferrer)
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return false;
            }
            if (m_ModuleToSpecifierMap.insert(std::make_pair(v8::Global<v8::Module>(isolate, inModule), inReferrer)).second == false)
            {
                return false;
            }
            if (m_SpecifierToModuleMap.insert(std::make_pair(inReferrer, v8::Global<v8::Module>(isolate, inModule))).second == false)
            {
                return false;
            }
            return true;
        }

        v8::MaybeLocal<v8::Module> JSModules::GetModuleBySpecifier(std::string inSpecifier)
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return v8::MaybeLocal<v8::Module>();
            }
            auto it = m_SpecifierToModuleMap.find(inSpecifier);
            if (it == m_SpecifierToModuleMap.end())
            {
                return v8::MaybeLocal<v8::Module>();
            }
            return it->second.Get(isolate);
        }

        std::string JSModules::GetSpecifierByModule(const v8::Global<v8::Module> &inModule)
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return std::string();
            }
            auto it = m_ModuleToSpecifierMap.find(v8::Global<v8::Module>(isolate, inModule));
            if (it == m_ModuleToSpecifierMap.end())
            {
                return std::string();
            }
            return it->second;
        }

        void JSModules::ResetModules()
        {
            m_SpecifierToModuleMap.clear();
            m_ModuleToSpecifierMap.clear();
        }

        void JSModules::SetupModulesCallbacks(v8::Isolate *inIsolate)
        {
            inIsolate->SetHostImportModuleDynamicallyCallback(JSModules::ImportModuleDynamically);
            inIsolate->SetHostInitializeImportMetaObjectCallback(JSModules::InitializeImportMetaObject);
        }

        bool JSModules::AddModuleRootPath(std::string inModuleName, std::string inPath)
        {
            std::filesystem::path path(inPath);
            auto it = s_ModuleRoots.emplace(inModuleName, inPath);
            return it.second;
        }

        void JSModules::RemoveModuleRootPath(std::string inModuleName)
        {
            s_ModuleRoots.erase(inModuleName);
        }

        std::filesystem::path JSModules::FindModuleRootPath(std::string inModule)
        {
            auto it = s_ModuleRoots.find(inModule);
            if (it != s_ModuleRoots.end())
            {
                return it->second;
            }

            return std::filesystem::path();
        }

        bool JSModules::DidModulePathEscapeRoot(std::filesystem::path inRootPath, std::filesystem::path inScriptPath)
        {
            std::filesystem::path rootCanonical = std::filesystem::canonical(inRootPath);
            std::filesystem::path fileCanonical = std::filesystem::canonical(inScriptPath);

            auto it = std::search(fileCanonical.begin(), fileCanonical.end(), rootCanonical.begin(), rootCanonical.end());

            return it != fileCanonical.begin();
        }

        bool JSModules::ParseImport(std::string &inImport, std::string &outModuleName, ImportType &outType)
        {
            if (inImport.empty())
            {
                return false;
            }

            size_t prefixDelimiter = inImport.find("://");
            if (prefixDelimiter == inImport.npos)
            {
                outType = ImportType::File;
            }
            else
            {
                std::string prefix = inImport.substr(0, prefixDelimiter);
                if (prefix == "file")
                {
                    //files will inherit their module's root
                    outType = ImportType::File;
                    //strip teh file:// prefix frmo the path
                    inImport = inImport.erase(0, prefixDelimiter + 3);
                }
                else if (prefix == "module")
                {
                    outType = ImportType::Module;
                    //need to extract the module name which is betwrrn the :// and the first /
                    inImport.erase(0, prefixDelimiter);
                    prefixDelimiter = inImport.find("/");
                    if (prefixDelimiter == inImport.npos)
                    {
                        Log::LogMessage msg;
                        msg.emplace(Log::MsgKey::Msg, "Failed to find module name missing / to start path");
                        msg.emplace(Log::MsgKey::File, inImport);
                        LOG_ERROR(msg);

                        return false;
                    }
                    outModuleName = outModuleName.substr(0, prefixDelimiter);
                    inImport.erase(0, prefixDelimiter);
                    //if there is no path then set the import to be main
                    if (inImport.empty())
                    {
                        inImport = "main";
                    }
                }
                else if (prefix == "http" || prefix == "https")
                {
                    outType = ImportType::Url;
                    //TODO: Figure out how to speift the name possibly the url goes to a manifest file for the module.
                }
                else
                {
                    Log::LogMessage msg;
                    msg.emplace(Log::MsgKey::Msg, "Unknown module prefix for import");
                    msg.emplace(Log::MsgKey::File, inImport);
                    LOG_ERROR(msg);

                    return false;
                }
            }
            return true;
        }

        void JSModules::RemoveAllRootPaths()
        {
            s_ModuleRoots.clear();
        }

        v8::MaybeLocal<v8::Module> JSModules::LoadModule(std::string inModuleName)
        {
            if (m_Context == nullptr)
            {
                return v8::MaybeLocal<v8::Module>();
            }

            v8::Isolate *isolate = m_Context->GetIsolate();
            v8::EscapableHandleScope handleScope(isolate);
            v8::Context::Scope contextScope(m_Context->GetContext());
            v8::TryCatch tryCatch(isolate);

            v8::Local<v8::Module> rootModule;
            std::filesystem::path rootPath;

            if (LoadModuleTree(m_Context, inModuleName, rootPath).ToLocal(&rootModule) == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Failed to load root module");
                msg.emplace(Log::MsgKey::File, inModuleName);
                if (tryCatch.HasCaught())
                {
                    msg.emplace(Log::MsgKey::StackTrace, JSUtilities::GetStackTrace(isolate, tryCatch));
                }
                LOG_ERROR(msg);
                return v8::MaybeLocal<v8::Module>();
            }

            return handleScope.Escape(rootModule);
        }

        bool JSModules::InstantiateModule(v8::Local<v8::Module> inModule)
        {
            if (inModule.IsEmpty())
            {
                return false;
            }

            return inModule->InstantiateModule(m_Context->GetContext(), ResolveModuleCallback).FromMaybe(false);
        }

        bool JSModules::RunModule(v8::Local<v8::Module> inModule)
        {
            if (inModule.IsEmpty())
            {
                return false;
            }
           v8::TryCatch tryCatch(m_Context->GetIsolate());

            inModule->Evaluate(m_Context->GetContext());
             if(tryCatch.HasCaught())
            {
                Log::LogMessage message = {
                    {Log::MsgKey::Msg, "Failed to run the module"},
                    {Log::MsgKey::StackTrace, JSUtilities::GetStackTrace(m_Context->GetIsolate(), tryCatch)}
                };
                LOG_ERROR(message);
                return false;
            }
            return false;
       }

        v8::MaybeLocal<v8::Promise> JSModules::ImportModuleDynamically(v8::Local<v8::Context> inContext, v8::Local<v8::ScriptOrModule> inReferrer,
                                                                       v8::Local<v8::String> inSpecifier)
        {
            v8::Isolate *isolate = inContext->GetIsolate();
            JSContext *jsContext = JSContext::GetJSContext(inContext);

            v8::MaybeLocal<v8::Promise::Resolver> maybeResolver = v8::Promise::Resolver::New(inContext);
            v8::Local<v8::Promise::Resolver> resolver;

            if (maybeResolver.ToLocal(&resolver) == false)
            {
                return v8::MaybeLocal<v8::Promise>();
            }

            internal::ModuleCallbackData *data = new internal::ModuleCallbackData(jsContext, v8::Local<v8::String>::Cast(inReferrer->GetResourceName()),
                                                                                  inSpecifier, resolver, v8::Local<v8::Value>());
            isolate->EnqueueMicrotask(JSModules::ImportModuleDynamicallyMicroTask, data);
            return resolver->GetPromise();
        }

        void JSModules::InitializeImportMetaObject(v8::Local<v8::Context> inContext, v8::Local<v8::Module> inModule, v8::Local<v8::Object> inMeta)
        {
            v8::Isolate *isolate = inContext->GetIsolate();
            v8::HandleScope handleScope(isolate);

            JSContext *jsContext = JSContext::GetJSContext(inContext);
            CHECK_NOT_NULL(jsContext);
            std::weak_ptr<JSModules> weakJSModule = jsContext->GetModules();
            CHECK_FALSE(weakJSModule.expired());
            JSModules *jsModule = weakJSModule.lock().get();

            std::string specifier = jsModule->GetSpecifierByModule(v8::Global<v8::Module>(isolate, inModule));

            v8::Local<v8::String> urlKey = JSUtilities::StringToV8(isolate, specifier, v8::NewStringType::kInternalized);
            v8::Local<v8::String> url = JSUtilities::StringToV8(isolate, specifier);
            if (url->Length() == 0)
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "URL specifier for module was empty");
                return;
            }

            if (inMeta->CreateDataProperty(inContext, urlKey, url).ToChecked() == false)
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::ReferenceError, "Failed to create the urlKey for the module's meta");
            }
        }

        void JSModules::ResolvePromiseCallback(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
        {
            std::unique_ptr<internal::ModuleCallbackData> importData(static_cast<internal::ModuleCallbackData *>(inInfo.Data().As<v8::External>()->Value()));
            v8::Isolate *isolate = importData->m_Context->GetIsolate();
            v8::Local<v8::Context> context = importData->m_Context->GetContext();

            v8::HandleScope handleScope(isolate);

            v8::Local<v8::Promise::Resolver> resolver(importData->m_Resolver.Get(isolate));
            v8::Local<v8::Value> moduleNamespace = importData->m_Namespace.Get(isolate);
            v8::Context::Scope contextScope(context);

            resolver->Resolve(context, moduleNamespace).ToChecked();
        }

        void JSModules::RejectPromiseCallback(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
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

        v8::MaybeLocal<v8::Module> JSModules::ResolveModuleCallback(v8::Local<v8::Context> inContext, v8::Local<v8::String> inSpecifier, v8::Local<v8::Module> inReferrer)
        {
            v8::Isolate *isolate = inContext->GetIsolate();
            JSContext *jsContext = JSContext::GetJSContext(inContext);

            std::weak_ptr<JSModules> weakJSModules = jsContext->GetModules();
            CHECK_FALSE(weakJSModules.expired());
            JSModules *jsModule = weakJSModules.lock().get();

            std::filesystem::path specifier = jsModule->GetSpecifierByModule(v8::Global<v8::Module>(isolate, inReferrer));
            if (specifier == "")
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Failed to ind the module");
                return v8::MaybeLocal<v8::Module>();
            }
            specifier.remove_filename();
            specifier /= JSUtilities::V8ToString(isolate, inSpecifier);

            v8::MaybeLocal<v8::Module> module = jsModule->GetModuleBySpecifier(specifier);
            if (module.IsEmpty())
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Failed to ind the module for " + specifier.filename().string());
                return v8::MaybeLocal<v8::Module>();
            }

            return module;
        }

        void JSModules::ImportModuleDynamicallyMicroTask(void *inData)
        {
            std::unique_ptr<internal::ModuleCallbackData> importData(static_cast<internal::ModuleCallbackData *>(inData));
            v8::Isolate *isolate = importData->m_Context->GetIsolate();
            v8::Local<v8::Context> context = importData->m_Context->GetContext();

            v8::HandleScope handleScope(isolate);

            std::weak_ptr<JSModules> weakJSModule = importData->m_Context->GetModules();
            CHECK_FALSE(weakJSModule.expired());
            JSModules *jsModule = weakJSModule.lock().get();

            v8::Local<v8::String> referrer = importData->m_Referrer.Get(isolate);
            v8::Local<v8::String> specifier = importData->m_Specifier.Get(isolate);
            v8::Local<v8::Promise::Resolver> resolver = importData->m_Resolver.Get(isolate);

            v8::Context::Scope contextScope(context);

            //TODO: Add support for loading modules from remote locations.
            std::string sourceUrl = JSUtilities::V8ToString(isolate, referrer);
            std::filesystem::path sourcePath(sourceUrl);
            if (sourcePath.is_absolute())
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Module path's can not be an absolute path: " + sourceUrl);
                return;
            }
            std::string fileName = JSUtilities::V8ToString(isolate, specifier);
            std::string fullFileName = JSModules::FindModuleRootPath(fileName);
            if (fullFileName.length() == 0)
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Failed to find module " + fileName);
                return;
            }

            v8::TryCatch tryCatch(isolate);
            tryCatch.SetVerbose(true);

            v8::Local<v8::Module> module = jsModule->GetModuleBySpecifier(fullFileName).FromMaybe(v8::Local<v8::Module>());
            if (module.IsEmpty())
            {
                if (LoadModuleTree(importData->m_Context, fullFileName, "").ToLocal(&module) == false)
                {
                    CHECK(tryCatch.HasCaught());
                    resolver->Reject(context, tryCatch.Exception()).ToChecked();
                    return;
                }
            }

            v8::MaybeLocal<v8::Value> maybeResult;
            if (module->InstantiateModule(context, JSModules::ResolveModuleCallback).FromMaybe(false) == false)
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
                    resolver->Reject(context, JSUtilities::StringToV8(isolate, "Expected promise to be returned from evaluated module " + fileName));
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

            internal::ModuleCallbackData *data = new internal::ModuleCallbackData(importData->m_Context, v8::Local<v8::String>(), v8::Local<v8::String>(), resolver, moduleNamespace);
            v8::Local<v8::External> eData = v8::External::New(isolate, data);
            v8::Local<v8::Function> callbackResolve;
            if (v8::Function::New(context, JSModules::ResolvePromiseCallback, eData).ToLocal(&callbackResolve) == false)
            {
                resolver->Reject(context, JSUtilities::StringToV8(isolate, "Failed to create the module resolver resolve callback function")).ToChecked();
                return;
            }
            v8::Local<v8::Function> callbackReject;
            if (v8::Function::New(context, JSModules::RejectPromiseCallback, eData).ToLocal(&callbackReject) == false)
            {
                resolver->Reject(context, JSUtilities::StringToV8(isolate, "Failed to create the module resolver reject callback function")).ToChecked();
                return;
            }
            resultPromise->Then(context, callbackResolve, callbackReject).ToLocalChecked();
        }

        v8::MaybeLocal<v8::Module> JSModules::LoadModuleTree(JSContext *inContext, const std::string &inImportPath, std::filesystem::path inModuleRoot)
        {
            v8::Isolate *isolate = inContext->GetIsolate();
            std::string moduleName;
            std::string importPathStr = inImportPath;
            ImportType importType;

            if (ParseImport(importPathStr, moduleName, importType) == false)
            {
                return v8::MaybeLocal<v8::Module>();
            }

            switch (importType)
            {
            case ImportType::File:
                //if the root path is empty then it's assumed to be rooted at app root path
                if (inModuleRoot.empty())
                {
                    moduleName = "app";
                    inModuleRoot = FindModuleRootPath(moduleName);
                }
                break;
            case ImportType::Module:
                inModuleRoot = FindModuleRootPath(moduleName);
                break;
            case ImportType::Url:
                //TODO: add being able to handle loading modules from the internet.
                return v8::MaybeLocal<v8::Module>();
            }

            if (inModuleRoot.empty())
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Failed to find the module file: " + inImportPath);
                return v8::MaybeLocal<v8::Module>();
            }

            std::filesystem::path importPath(inModuleRoot);
            importPath /= importPathStr;

            std::weak_ptr<JSModules> weakModules = inContext->GetModules();
            CHECK_FALSE(weakModules.expired());
            JSModules *modules = weakModules.lock().get();

            //check to see if the module hasn't been loaded already
            auto it = modules->m_SpecifierToModuleMap.find(importPath);
            if (it != modules->m_SpecifierToModuleMap.end())
            {
                return it->second.Get(isolate);
            }

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

            v8::ScriptOrigin origin(JSUtilities::StringToV8(isolate, importPath), v8::Local<v8::Integer>(), v8::Local<v8::Integer>(),
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
                //Load module tree will determine after parsing the import if the module is already loaded.
                if (LoadModuleTree(inContext, importPathStr, inModuleRoot).IsEmpty())
                {
                    return v8::MaybeLocal<v8::Module>();
                }
            }
            return module;
        }

    }
}