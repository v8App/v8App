// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <algorithm>
#include <regex>

#include "Assets/AppAssetRoots.h"
#include "Assets/TextAsset.h"
#include "Logging/LogMacros.h"
#include "Utils/Format.h"
#include "Utils/Paths.h"

#include "CodeCache.h"
#include "JSContext.h"
#include "JSContextModules.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace internal
        {
            struct ModuleCallbackData
            {
                JSContextSharedPtr m_Context;
                V8GlobalPromiseResolver m_Resolver;
                JSModuleInfoSharedPtr m_ModuleInfo;

                ModuleCallbackData(JSContextSharedPtr inContext, JSModuleInfoSharedPtr inModuleInfo,
                                   V8LocalPromiseResolver &inResolver)
                {
                    V8Isolate *isolate = inContext->GetIsolate();
                    m_Context = inContext;
                    m_Resolver.Reset(isolate, inResolver);
                    m_ModuleInfo = inModuleInfo;
                }
            };
            struct ModuleResolutionData
            {
                V8GlobalPromiseResolver m_Resolver;
                V8GlobalValue m_Namespace;
                JSContextSharedPtr m_Context;

                ModuleResolutionData(JSContextSharedPtr inContext, V8LocalPromiseResolver inResolver, V8LocalValue inNamespace)
                {
                    V8Isolate *isolate = inContext->GetIsolate();
                    m_Context = inContext;
                    m_Resolver.Reset(isolate, inResolver);
                    m_Namespace.Reset(isolate, inNamespace);
                }
            };
        } // internal

        JSContextModules::JSContextModules(JSContextSharedPtr inContext) : m_Context(inContext)
        {
            CHECK_NE(inContext, nullptr);
        }

        JSContextModules::~JSContextModules()
        {
            m_Context = nullptr;
        }

        V8Isolate *JSContextModules::GetIsolate() const
        {
            return m_Context == nullptr ? nullptr : m_Context->GetIsolate();
        }

        JSModuleInfoSharedPtr JSContextModules::LoadModule(std::filesystem::path inModulePath)
        {
            V8Isolate *isolate = m_Context->GetIsolate();
            if (isolate == nullptr)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("LoadModule got a null isolate loading {}", inModulePath));
                LOG_ERROR(msg);
                return nullptr;
            }

            Assets::AppAssetRootsSharedPtr appRoot = m_Context->GetJSRuntime()->GetApp()->GetAppRoots();

            V8Isolate::Scope iScope(isolate);
            v8::EscapableHandleScope handleScope(isolate);
            v8::TryCatch tryCatch(isolate);
            v8::Context::Scope contextScope(m_Context->GetLocalContext());

            JSModuleInfo::AssertionInfo assertInfo;
            if (".json" == inModulePath.extension().string())
            {
                assertInfo.m_Type = JSModuleInfo::ModuleType::kJSON;
            }
            else
            {
                assertInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            }
            JSModuleInfoSharedPtr info = BuildModuleInfo(assertInfo, inModulePath, appRoot->GetAppRoot());
            if (info == nullptr)
            {
                return nullptr;
            }
            JSModuleInfoSharedPtr cached = GetModuleBySpecifier(inModulePath);
            if (cached != nullptr)
            {
                return cached;
            }

            V8LocalModule rootModule;
            return LoadModuleTree(m_Context, info);
        }

        bool JSContextModules::InstantiateModule(JSModuleInfoSharedPtr inModule)
        {
            if (inModule == nullptr)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "InstantiateModule passed a null module ptr"},
                };
                LOG_ERROR(msg);
                return false;
            }
            V8LocalModule module = inModule->GetLocalModule();
            if (module.IsEmpty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "InstantiateModule passed module info's module is empty"},
                };
                LOG_ERROR(msg);
                return false;
            }
            V8Isolate *isolate = m_Context->GetIsolate();

            v8::TryCatch tryCatch(isolate);
            if (module->InstantiateModule(m_Context->GetLocalContext(), ResolveModuleCallback).FromMaybe(false) == false)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to instantiate module: {}", inModule->GetModulePath())},
                };
                if (tryCatch.HasCaught())
                {
                    msg.emplace(Log::MsgKey::StackTrace, JSUtilities::GetStackTrace(m_Context->GetLocalContext(), tryCatch));
                }
                LOG_ERROR(msg);
                return false;
            }
            return true;
            // TODO: finish code cache code after understading lifecycle
            /** CodeCacheSharedPtr cache = m_Context->GetJSRuntime()->GetApp()->GetCodeCache();
            if (cache->HasCodeCache(inModule->GetModulePath()))
            {
                return true;
            }
            V8LocalUnboundModuleScript unbound = inModule->GetLocalModule()->GetUnboundModuleScript();
            inModule->SetUnboundScript(unbound);
            return true;
            **/
        }

        bool JSContextModules::RunModule(JSModuleInfoSharedPtr inModule)
        {
            if (inModule == nullptr)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "RunModule passed a null module ptr"},
                };
                LOG_ERROR(msg);
                return false;
            }
            V8LocalModule module = inModule->GetLocalModule();
            if (module.IsEmpty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "RunModule passed module info's module is empty"},
                };
                LOG_ERROR(msg);
                return false;
            }
            V8Isolate *isolate = m_Context->GetIsolate();
            v8::TryCatch tryCatch(m_Context->GetIsolate());

            // TODO: handle the returned value
            module->Evaluate(m_Context->GetLocalContext());
            if (tryCatch.HasCaught())
            {
                Log::LogMessage message = {
                    {Log::MsgKey::Msg, "Failed to run the module"},
                    {Log::MsgKey::StackTrace, JSUtilities::GetStackTrace(m_Context->GetLocalContext(), tryCatch)}};
                LOG_ERROR(message);
                return false;
            }
            return true;
        }

        JSModuleInfoSharedPtr JSContextModules::GetModuleBySpecifier(std::string inSpecifier)
        {
            V8Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return JSModuleInfoSharedPtr();
            }
            for (auto it : m_ModuleMap)
            {
                if (it.first.first == inSpecifier)
                {
                    return it.second;
                }
            }
            return nullptr;
        }

        std::string JSContextModules::GetSpecifierByModule(V8LocalModule inModule)
        {
            JSModuleInfoSharedPtr info = GetModuleInfoByModule(inModule, JSModuleInfo::ModuleType::kInvalid);
            if (info == nullptr)
            {
                return std::string();
            }
            return info->GetModulePath();
        }

        V8LocalValue JSContextModules::GetJSONByModule(V8LocalModule inModule)
        {
            JSModuleInfoSharedPtr info = GetModuleInfoByModule(inModule, JSModuleInfo::ModuleType::kJSON);
            if (info == nullptr)
            {
                return V8LocalValue();
            }
            return info->GetLocalJSON();
        }

        void JSContextModules::ResetModules()
        {
            m_ModuleMap.clear();
        }

        void JSContextModules::SetupModulesCallbacks(V8Isolate *inIsolate)
        {
            inIsolate->SetHostImportModuleDynamicallyCallback(JSContextModules::HostImportModuleDynamically);
            inIsolate->SetHostInitializeImportMetaObjectCallback(JSContextModules::InitializeImportMetaObject);
        }

        bool JSContextModules::AddModule(const JSModuleInfoSharedPtr &inModule, std::string inFileName, JSModuleInfo::ModuleType inModuleType)
        {
            V8Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return false;
            }
            if (m_ModuleMap.insert(std::make_pair(std::make_pair(inFileName, inModuleType), inModule)).second == false)
            {
                return false;
            }
            return true;
        }

        JSModuleInfoSharedPtr JSContextModules::GetModuleInfoByModule(V8LocalModule inModule, JSModuleInfo::ModuleType inType)
        {
            V8GlobalModule globMod(m_Context->GetIsolate(), inModule);
            for (auto it : m_ModuleMap)
            {
                if (it.second->GetLocalModule()->GetIdentityHash() == globMod.Get(m_Context->GetIsolate())->GetIdentityHash())
                {
                    if (inType == JSModuleInfo::ModuleType::kInvalid)
                    {
                        return it.second;
                    }
                    else if (it.second->GetAssertionInfo().m_Type == inType)
                    {
                        return it.second;
                    }
                }
            }
            return nullptr;
        }

        JSModuleInfo::AssertionInfo JSContextModules::GetModuleAssertionInfo(JSContextSharedPtr inContext, V8LocalFixedArray inAssertions, bool inHasPostions)
        {
            V8Isolate *isolate = inContext->GetIsolate();
            const int kEntrySize = inHasPostions ? 3 : 2;
            JSModuleInfo::AssertionInfo returnInfo;
            returnInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            returnInfo.m_TypeString = "js";
            V8LocalContext context = inContext->GetLocalContext();

            if (isolate == nullptr)
            {
                // TODO:: Log message
                returnInfo.m_Type = JSModuleInfo::ModuleType::kInvalid;
                returnInfo.m_TypeString = "";
                return returnInfo;
            }
            for (int i = 0; i < inAssertions->Length(); i += kEntrySize)
            {
                v8::Local<v8::String> v8Key = inAssertions->Get(context, i).As<v8::String>();
                std::string key = JSUtilities::V8ToString(isolate, v8Key);
                // for some reason we can end up with a blank key
                if (key == "")
                {
                    continue;
                    ;
                }
                if (key == "type" || key == "module")
                {
                    v8::Local<v8::String> v8Value = inAssertions->Get(context, i + 1).As<v8::String>();
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
                            Log::LogMessage message = {
                                {Log::MsgKey::Msg, Utils::format("Unknown type assertion: {}", value)},
                            };
                            LOG_WARN(message);

                            returnInfo.m_Type = JSModuleInfo::ModuleType::kInvalid;
                            returnInfo.m_TypeString = "";
                        }
                    }
                    else if (key == "module")
                    {
                        returnInfo.m_Module = value;
                    }
                }
                else
                {
                    Log::LogMessage message = {
                        {Log::MsgKey::Msg, Utils::format("Unknown assertion: {}", key)},
                    };
                    LOG_WARN(message);
                }
            }

            return returnInfo;
        }

        JSModuleInfoSharedPtr JSContextModules::BuildModuleInfo(JSModuleInfo::AssertionInfo &inAssertionInfo, const std::filesystem::path &inImportPath, const std::filesystem::path &inCurrentModPath)
        {
            std::filesystem::path absImportPath = Utils::NormalizePath(inImportPath);
            Assets::AppAssetRootsSharedPtr appRoots = m_Context->GetJSRuntime()->GetApp()->GetAppRoots();
            V8Isolate *ioslate = m_Context->GetIsolate();
            JSModuleInfoSharedPtr moduleInfo = std::make_shared<JSModuleInfo>(m_Context);
            moduleInfo->SetAssertionInfo(inAssertionInfo);

            const char startChar = absImportPath.c_str()[0];
            // may start with a token so let the app root handle it
            if (startChar == '%')
            {
                absImportPath = appRoots->MakeAbsolutePathToAppRoot(absImportPath);
            }
            // absolute path from the app root since nothing can be imported outside of the app root
            else if (startChar == '/')
            {
                absImportPath = appRoots->MakeAbsolutePathToAppRoot(absImportPath);
            }
            else
            {
                if (inAssertionInfo.m_Module.empty())
                {
                    absImportPath = Utils::MakeAbsolutePathToRoot(absImportPath, inCurrentModPath);
                }
                else
                {
                    std::filesystem::path modPath = appRoots->FindModuleLatestVersionRootPath(inAssertionInfo.m_Module);

                    if (modPath.empty())
                    {
                        JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::SyntaxError,
                                                  Utils::format("Failed to find asserted module: {}, ImportPath: {}", inAssertionInfo.m_Module, inImportPath));
                        return nullptr;
                    }
                    absImportPath = Utils::MakeAbsolutePathToRoot(absImportPath, modPath);
                }
            }
            // now make it a relative path to the app root
            std::filesystem::path relModulePath = appRoots->MakeRelativePathToAppRoot(absImportPath);
            auto it = relModulePath.begin();

            std::string module = it->string();

            // located in the js folder
            if (module == Assets::c_RootJS)
            {
                if (inAssertionInfo.m_Module.empty() == false)
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::SyntaxError,
                                              Utils::format("Can not use a module assetion in the {} root, ImportPath: {}", Assets::c_RootJS, inImportPath));
                    return nullptr;
                }
                if (inAssertionInfo.DoesExtensionMatchType(relModulePath.extension().string()) == false)
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::TypeError,
                                              Utils::format("File type doesn't match specified type {}. Importpath: {}",
                                                            inAssertionInfo.m_TypeString, inImportPath));
                    return nullptr;
                }
                moduleInfo->SetPath(absImportPath);
                // files in the js just use the filename for the module name
                moduleInfo->SetName(absImportPath.stem().string());
                return moduleInfo;
            }
            else if (module == Assets::c_RootModules)
            {
                // ok we are doing a module.
                if (inAssertionInfo.DoesExtensionMatchType(relModulePath.extension().string()) == false)
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::TypeError,
                                              Utils::format("File type doesn't match specified type {}. Importpath: {}",
                                                            inAssertionInfo.m_TypeString, inImportPath));
                    return nullptr;
                }

                it++;
                module = it->string();

                if (inAssertionInfo.m_Module.empty() == false && inAssertionInfo.m_Module != module)
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::SyntaxError,
                                              Utils::format("Import path was not in asserted module's path. Module:{}, ImportPath: {}",
                                                            inAssertionInfo.m_Module, inImportPath));
                    return nullptr;
                }

                moduleInfo->SetName(module);
                it++;
                std::string versionStr = it->string();
                it++;
                Utils::VersionString version(versionStr);
                if (version.IsVersionString() == false)
                {
                    it--;
                    version = appRoots->GetModulesLatestVersion(module);
                    versionStr = version.GetVersionString();
                    if (versionStr == "")
                    {
                        JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::SyntaxError,
                                                  Utils::format("Failed to find module's version: {}, ImportPath: {}", module, inImportPath));
                        return nullptr;
                    }
                }
                moduleInfo->SetVersion(versionStr);
                module += "/" + versionStr;

                std::filesystem::path modPath = appRoots->FindModuleVersionRootPath(module);
                if (modPath.empty())
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::SyntaxError,
                                              Utils::format("Failed to find module: {}, ImportPath: {}", module, inImportPath));
                    return nullptr;
                }

                absImportPath = modPath;
                for (; it != relModulePath.end(); it++)
                {
                    absImportPath /= *it;
                }
                moduleInfo->SetPath(absImportPath);
                return moduleInfo;
            }
            else if (module == Assets::c_RootResource)
            {
                if (inAssertionInfo.m_Module.empty() == false)
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::SyntaxError,
                                              Utils::format("Can not use a module assetion in the {} root, ImportPath: {}",
                                                            Assets::c_RootResource, inImportPath));
                    return nullptr;
                }
                std::string ext = relModulePath.extension().string();
                if (ext == ".js" || ext == ".mjs")
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::SyntaxError,
                                              Utils::format("Files ending in .js or .mjs can not be in resources, ImportPath: {}", inImportPath));
                    return nullptr;
                }
                if (inAssertionInfo.DoesExtensionMatchType(ext) == false)
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::TypeError,
                                              Utils::format("File type doesn't match specified type {}. Importpath: {}",
                                                            inAssertionInfo.m_TypeString, inImportPath));
                    return nullptr;
                }
                moduleInfo->SetPath(absImportPath);
                // files in the resources just use the filename for the module name
                moduleInfo->SetName(absImportPath.stem().string());

                return moduleInfo;
            }
            return nullptr;
        }

        V8MaybeLocalPromise JSContextModules::HostImportModuleDynamically(V8LocalContext inContext,
                                                                          V8LocalData inDefinedOptions,
                                                                          V8LocalValue inResourceName,
                                                                          V8LocalString inSpecifier,
                                                                          V8LocalFixedArray importAssertions)
        {
            V8Isolate *isolate = inContext->GetIsolate();
            v8::MaybeLocal<v8::Promise::Resolver> maybeResolver = v8::Promise::Resolver::New(inContext);
            v8::Local<v8::Promise::Resolver> resolver;

            if (maybeResolver.ToLocal(&resolver) == false)
            {
                return V8MaybeLocalPromise();
            }

            if (inResourceName->IsNull())
            {
                resolver->Reject(inContext, v8::Exception::TypeError(v8::String::NewFromUtf8Literal(isolate, "Resource name is null"))).ToChecked();
                return resolver->GetPromise();
            }

            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);
            if (jsContext == nullptr)
            {
                resolver->Reject(inContext, v8::Exception::TypeError(JSUtilities::StringToV8(isolate, "Failed to get the JSContext from the v8Context"))).ToChecked();
                return resolver->GetPromise();
            }
            JSContextModulesSharedPtr jsModules = jsContext->GetJSModules();

            JSModuleInfo::AssertionInfo assertInfo = jsModules->GetModuleAssertionInfo(jsContext, importAssertions, true);
            std::filesystem::path resourceName = std::filesystem::path(JSUtilities::V8ToString(isolate, inResourceName));
            resourceName.remove_filename();
            std::filesystem::path specifier = std::filesystem::path(JSUtilities::V8ToString(isolate, inSpecifier));
            JSModuleInfoSharedPtr info = jsModules->BuildModuleInfo(assertInfo, specifier, resourceName);

            if (assertInfo.m_Type == JSModuleInfo::ModuleType::kInvalid)
            {
                resolver->Reject(inContext, JSUtilities::StringToV8(isolate, Utils::format("Import '{}' had an invalid type of '{}'", specifier, assertInfo.m_TypeString))).ToChecked();
                return resolver->GetPromise();
            }
            internal::ModuleCallbackData *data = new internal::ModuleCallbackData(jsContext, info, resolver);
            isolate->EnqueueMicrotask(JSContextModules::ImportModuleDynamicallyMicroTask, data);
            return resolver->GetPromise();
        }

        void JSContextModules::ImportModuleDynamicallyMicroTask(void *inData)
        {
            std::unique_ptr<internal::ModuleCallbackData> importData(static_cast<internal::ModuleCallbackData *>(inData));
            if (importData == nullptr)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "Failed to get teh import data for importing module"}};
                LOG_ERROR(msg);
                return;
            }
            if (importData->m_Context == nullptr)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "context shared point was null for importing module"}};
                LOG_ERROR(msg);
                return;
            }
            JSContextSharedPtr jsContext = importData->m_Context;
            V8Isolate *isolate = jsContext->GetIsolate();
            if (importData->m_Resolver.IsEmpty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "promise resolver was empty for importing module"}};
                LOG_ERROR(msg);
                return;
            }

            V8LocalPromiseResolver resolver = importData->m_Resolver.Get(isolate);
            JSModuleInfoSharedPtr importInfo = importData->m_ModuleInfo;
            JSContextModulesSharedPtr jsModules = jsContext->GetJSModules();

            v8::HandleScope iScope(isolate);
            V8LocalContext context = jsContext->GetLocalContext();
            v8::Context::Scope cScope(context);
            v8::TryCatch tryCatch(isolate);

            JSModuleInfoSharedPtr info = jsModules->GetModuleBySpecifier(importInfo->GetModulePath());
            if (info == nullptr)
            {
                info = LoadModuleTree(jsContext, importInfo);
                if (info == nullptr)
                {
                    resolver->Reject(context, JSUtilities::StringToV8(isolate, Utils::format("Failed to load module: {}", importInfo->GetModulePath())));
                    return;
                }
            }
            V8LocalModule module = info->GetLocalModule();
            v8::MaybeLocal<v8::Value> maybeResult;
            if (module->InstantiateModule(context, JSContextModules::ResolveModuleCallback).FromMaybe(false) == false)
            {
                CHECK(tryCatch.HasCaught());
                resolver->Reject(context, tryCatch.Exception()).ToChecked();
                return;
            }

            maybeResult = module->Evaluate(importData->m_Context->GetLocalContext());
            if (maybeResult.IsEmpty() == false)
            {
                v8::MicrotasksScope::PerformCheckpoint(isolate);
            }

            V8LocalValue result;
            if (maybeResult.ToLocal(&result) == false)
            {
                DCHECK_TRUE(tryCatch.HasCaught());
                resolver->Reject(context, tryCatch.Exception()).ToChecked();
                return;
            }

            V8LocalValue moduleNamespace = module->GetModuleNamespace();
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
            v8::Local<v8::Promise> resultPromise(result.As<v8::Promise>());
            if (resultPromise->State() == v8::Promise::kRejected)
            {
                resolver->Reject(context, resultPromise->Result()).ToChecked();
                return;
            }
            internal::ModuleResolutionData *data = new internal::ModuleResolutionData(jsContext, resolver, moduleNamespace);
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

        void JSContextModules::InitializeImportMetaObject(V8LocalContext inContext, V8LocalModule inModule, V8LocalObject inMeta)
        {
            V8Isolate *isolate = inContext->GetIsolate();
            v8::HandleScope handleScope(isolate);

            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);
            CHECK_NOT_NULL(jsContext);
            JSContextModulesSharedPtr jsModules = jsContext->GetJSModules();
            CHECK_NOT_NULL(jsModules);

            JSModuleInfoSharedPtr info = jsModules->GetModuleInfoByModule(inModule);
            CHECK_NOT_NULL(info);

            V8LocalString urlKey = JSUtilities::StringToV8(isolate, "url");
            V8LocalString url = JSUtilities::StringToV8(isolate, info->GetModulePath());
            inMeta->CreateDataProperty(inContext, urlKey, url);
        }

        V8MaybeLocalValue JSContextModules::JSONEvalutionSteps(V8LocalContext inContext, V8LocalModule inModule)
        {
            V8LocalPromiseResolver resolver = v8::Promise::Resolver::New(inContext).ToLocalChecked();
            V8Isolate *isolate = inContext->GetIsolate();

            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);
            if (jsContext == nullptr)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Failed to get the JSContext from the v8Context");
                LOG_ERROR(msg);
                resolver->Reject(inContext, v8::Undefined(isolate));
                return resolver->GetPromise();
            }
            JSContextModulesSharedPtr jsModule = jsContext->GetJSModules();
            if (jsModule == nullptr)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Failed to get the JSContextModules from the JSCotext");
                LOG_ERROR(msg);
                resolver->Reject(inContext, v8::Undefined(isolate));
                return resolver->GetPromise();
            }

            V8LocalValue parsedJSON = jsModule->GetJSONByModule(inModule);
            if (parsedJSON.IsEmpty())
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Failed to find the module in the map");
                LOG_ERROR(msg);
                resolver->Reject(inContext, v8::Undefined(isolate));
                return resolver->GetPromise();
            }

            v8::TryCatch tryCatch(isolate);
            v8::Maybe<bool> result = inModule->SetSyntheticModuleExport(isolate,
                                                                        JSUtilities::StringToV8(isolate,
                                                                                                "default",
                                                                                                v8::NewStringType::kInternalized),
                                                                        parsedJSON);
            if (tryCatch.HasCaught())
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, JSUtilities::GetStackTrace(inContext, tryCatch));
                LOG_ERROR(msg);
                resolver->Reject(inContext, v8::Undefined(isolate));
                return resolver->GetPromise();
            }

            if (result.IsNothing() || result.FromJust() == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Failed to set the default export for the json module");
                LOG_ERROR(msg);
                resolver->Reject(inContext, v8::Undefined(isolate));
                return resolver->GetPromise();
            }

            resolver->Resolve(inContext, v8::Undefined(isolate));
            return resolver->GetPromise();
        }

        void JSContextModules::ResolvePromiseCallback(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
        {
            std::unique_ptr<internal::ModuleResolutionData> importData(static_cast<internal::ModuleResolutionData *>(inInfo.Data().As<v8::External>()->Value()));
            V8Isolate *isolate = importData->m_Context->GetIsolate();
            v8::Local<v8::Context> context = importData->m_Context->GetLocalContext();
            v8::HandleScope hScope(isolate);

            v8::Local<v8::Promise::Resolver> resolver(importData->m_Resolver.Get(isolate));
            V8LocalValue moduleNamespace = importData->m_Namespace.Get(isolate);
            v8::Context::Scope cScope(context);

            resolver->Resolve(context, moduleNamespace).ToChecked();
        }

        void JSContextModules::RejectPromiseCallback(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
        {
            std::unique_ptr<internal::ModuleCallbackData> importData(static_cast<internal::ModuleCallbackData *>(inInfo.Data().As<v8::External>()->Value()));
            V8Isolate *isolate = importData->m_Context->GetIsolate();
            v8::Local<v8::Context> context = importData->m_Context->GetLocalContext();
            v8::HandleScope hScope(isolate);

            v8::Local<v8::Promise::Resolver> resolver(importData->m_Resolver.Get(isolate));
            v8::Context::Scope cScope(context);

            DCHECK_EQ(inInfo.Length(), 1);
            resolver->Reject(context, inInfo[0]).ToChecked();
        }

        V8MaybeLocalModule JSContextModules::ResolveModuleCallback(v8::Local<v8::Context> inContext, v8::Local<v8::String> inSpecifier, v8::Local<v8::FixedArray> inAssertions, V8LocalModule inReferrer)
        {
            V8Isolate *isolate = inContext->GetIsolate();
            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);

            JSContextModulesSharedPtr jsModule = jsContext->GetJSModules();
            std::filesystem::path filePath = std::filesystem::path(JSUtilities::V8ToString(isolate, inSpecifier));

            std::filesystem::path specifier = jsModule->GetSpecifierByModule(inReferrer);

            JSModuleInfo::AssertionInfo assertInfo = jsModule->GetModuleAssertionInfo(jsContext, inAssertions, false);
            if (assertInfo.m_Type == JSModuleInfo::ModuleType::kInvalid)
            {
                return V8MaybeLocalModule();
            }

            specifier.remove_filename();
            JSModuleInfoSharedPtr moduleInfo = jsModule->BuildModuleInfo(assertInfo, filePath, specifier);
            if (moduleInfo == nullptr)
            {
                return V8MaybeLocalModule();
            }

            moduleInfo = jsModule->GetModuleBySpecifier(moduleInfo->GetModulePath());
            if (moduleInfo == nullptr)
            {
                return V8MaybeLocalModule();
            }
            return moduleInfo->GetLocalModule();
        }

        JSModuleInfoSharedPtr JSContextModules::LoadModuleTree(JSContextSharedPtr inContext, const JSModuleInfoSharedPtr inModuleInfo)
        {
            V8Isolate *isolate = inContext->GetIsolate();
            V8LocalContext context = inContext->GetLocalContext();
            JSAppSharedPtr app = inContext->GetJSRuntime()->GetApp();
            Assets::AppAssetRootsSharedPtr appRoot = inContext->GetJSRuntime()->GetApp()->GetAppRoots();
            JSContextModulesSharedPtr jsModule = inContext->GetJSModules();

            JSModuleInfo::ModuleType moduleType = inModuleInfo->GetAssertionInfo().m_Type;
            std::filesystem::path importPath = inModuleInfo->GetModulePath();
            V8LocalModule module;

            if (moduleType == JSModuleInfo::ModuleType::kJavascript)
            {
                V8ScriptSourceUniquePtr source = app->GetCodeCache()->LoadScriptFile(importPath, isolate);
                if (source == nullptr)
                {
                    JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, Utils::format("Failed to load the module file: {}", importPath));
                    Log::LogMessage msg;
                    msg.emplace(Log::MsgKey::Msg, Utils::format("Failed to load the module file: {}", importPath));
                    LOG_ERROR(msg);
                    return nullptr;
                }
                v8::TryCatch tryCatch(isolate);
                v8::ScriptCompiler::CompileOptions options = v8::ScriptCompiler::kNoCompileOptions;
                if (source->GetCachedData() != nullptr)
                {
                    options = v8::ScriptCompiler::kConsumeCodeCache;
                }
                V8MaybeLocalModule maybeModule = v8::ScriptCompiler::CompileModule(isolate, source.get(), options);
                if (tryCatch.HasCaught())
                {
                    tryCatch.ReThrow();
                    Log::LogMessage msg;
                    msg.emplace(Log::MsgKey::Msg, JSUtilities::GetStackTrace(context, tryCatch));
                    LOG_ERROR(msg);
                    return nullptr;
                }
                module = maybeModule.ToLocalChecked();
                inModuleInfo->SetV8Module(module);
            }
            else if (moduleType == JSModuleInfo::ModuleType::kJSON)
            {
                V8LocalString jsonStr;
                {
                    // TODO:: Create an asset cache for non script files
                    Assets::TextAsset file(importPath);
                    if (file.ReadAsset() == false)
                    {
                        JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, Utils::format("Failed to load the module file: {}", importPath));
                        Log::LogMessage msg;
                        msg.emplace(Log::MsgKey::Msg, Utils::format("Failed to load the module file: {}", importPath));
                        LOG_ERROR(msg);
                        return nullptr;
                    }
                    jsonStr = JSUtilities::StringToV8(isolate, file.GetContent());
                }
                V8LocalValue parsedJSON;
                v8::TryCatch tryCatch(isolate);
                if (v8::JSON::Parse(context, jsonStr).ToLocal(&parsedJSON) == false)
                {
                    tryCatch.ReThrow();
                    Log::LogMessage msg;
                    msg.emplace(Log::MsgKey::Msg, JSUtilities::GetStackTrace(context, tryCatch, importPath.string()));
                    LOG_ERROR(msg);
                    return nullptr;
                }
                std::vector<V8LocalString> exportNames{JSUtilities::StringToV8(isolate, "default")};
                module = v8::Module::CreateSyntheticModule(isolate, JSUtilities::StringToV8(isolate, importPath), exportNames, JSContextModules::JSONEvalutionSteps);
                inModuleInfo->SetV8Module(module);
                inModuleInfo->SetV8JSON(parsedJSON);
            }
            else
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Uknown module type {}", JSModuleInfo::ModuleTypeToString(moduleType)));
                LOG_ERROR(msg);

                UNREACHABLE();
            }

            if (jsModule->AddModule(inModuleInfo, importPath, moduleType) == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Failed to add module into map. File: {}", importPath));
                LOG_ERROR(msg);
                return nullptr;
            }

            V8LocalFixedArray requests = module->GetModuleRequests();
            for (int x = 0, length = requests->Length(); x < length; x++)
            {
                V8LocalModuleRequst request = requests->Get(context, x).As<v8::ModuleRequest>();
                V8LocalString moduleName = request->GetSpecifier();
                V8LocalFixedArray v8Assertions = request->GetImportAssertions();

                JSModuleInfo::AssertionInfo assertInfo = jsModule->GetModuleAssertionInfo(inContext, v8Assertions, false);
                if (assertInfo.m_Type == JSModuleInfo::ModuleType::kInvalid)
                {
                    return nullptr;
                }
                std::filesystem::path requestPath(JSUtilities::V8ToString(isolate, moduleName));
                if (importPath.empty())
                {
                    return nullptr;
                }
                std::filesystem::path modPath = inModuleInfo->GetModulePath();
                modPath.remove_filename();
                JSModuleInfoSharedPtr moduleInfo = jsModule->BuildModuleInfo(assertInfo, requestPath, modPath);
                if (moduleInfo == nullptr)
                {
                    return nullptr;
                }
                JSModuleInfoSharedPtr cached = jsModule->GetModuleBySpecifier(moduleInfo->GetModulePath());
                if (cached != nullptr)
                {
                    continue;
                }
                if (LoadModuleTree(inContext, moduleInfo) == nullptr)
                {
                    return nullptr;
                }
            }

            return inModuleInfo;
        }
    } // JSRuntime
} // JSApp