// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
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
                V8GPromiseResolver m_Resolver;
                JSModuleInfoSharedPtr m_ModuleInfo;

                ModuleCallbackData(JSContextSharedPtr inContext, JSModuleInfoSharedPtr inModuleInfo,
                                   V8LPromiseResolver &inResolver)
                {
                    V8Isolate *isolate = inContext->GetIsolate();
                    m_Context = inContext;
                    m_Resolver.Reset(isolate, inResolver);
                    m_ModuleInfo = inModuleInfo;
                }
            };
            struct ModuleResolutionData
            {
                V8GPromiseResolver m_Resolver;
                V8GValue m_Namespace;
                JSContextSharedPtr m_Context;

                ModuleResolutionData(JSContextSharedPtr inContext, V8LPromiseResolver inResolver, V8LValue inNamespace)
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
                LOG_ERROR(Utils::format("LoadModule got a null isolate loading {}", inModulePath));
                return nullptr;
            }

            Assets::AppAssetRootsSharedPtr appRoot = m_Context->GetJSRuntime()->GetApp()->GetAppRoot();

            V8Isolate::Scope iScope(isolate);
            v8::EscapableHandleScope handleScope(isolate);
            V8TryCatch tryCatch(isolate);
            V8ContextScope contextScope(m_Context->GetLocalContext());

            JSModuleAttributesInfo attributesInfo;
            std::string ext =inModulePath.extension().string();
            if (JSModuleAttributesInfo::kExtJS == ext || JSModuleAttributesInfo::kExtModuleJS == ext)
            {
                attributesInfo.m_Type = JSModuleType::kJavascript;
            }
            else if (JSModuleAttributesInfo::kExtNative == ext)
            {
                attributesInfo.m_Type = JSModuleType::kNative;
            }
            else if (JSModuleAttributesInfo::kExtJSON == ext)
            {
                attributesInfo.m_Type = JSModuleType::kJSON;
            }
            else
            {
                LOG_ERROR(Utils::format("Unsupported extension for LoadModule: {}", inModulePath.extension().string()));
                return nullptr;
            }

            JSModuleInfoSharedPtr info = BuildModuleInfo(attributesInfo, inModulePath, appRoot->GetAppRoot());
            if (info == nullptr)
            {
                return nullptr;
            }
            JSModuleInfoSharedPtr cached = GetModuleBySpecifier(inModulePath.generic_string());
            if (cached != nullptr)
            {
                return cached;
            }

            return LoadModuleTree(m_Context, info);
        }

        bool JSContextModules::InstantiateModule(JSModuleInfoSharedPtr inModule)
        {
            if (inModule == nullptr)
            {
                LOG_ERROR("InstantiateModule passed a null module ptr");
                return false;
            }
            V8LModule module = inModule->GetLocalModule();
            if (module.IsEmpty())
            {
                LOG_ERROR("InstantiateModule passed module info's module is empty");
                return false;
            }
            V8Isolate *isolate = m_Context->GetIsolate();

            V8TryCatch tryCatch(isolate);
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
        }

        V8LValue JSContextModules::RunModule(JSModuleInfoSharedPtr inModule)
        {
            V8Isolate *isolate = m_Context->GetIsolate();
            // We should probably actually raise an error or excpetion to propgate up to the caller
            if (inModule == nullptr)
            {
                LOG_ERROR("RunModule passed a null module ptr");
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, "Failed to run module becuase a nullptr was passed for it");
                return V8LValue();
            }
            V8LModule module = inModule->GetLocalModule();
            if (module.IsEmpty())
            {
                LOG_ERROR("RunModule passed module info's module is empty");
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, Utils::format("Failed to run module: {}", inModule->GetName()));
                return V8LValue();
            }
            V8TryCatch tryCatch(m_Context->GetIsolate());

            // TODO: handle the returned value
            V8LValue result = module->Evaluate(m_Context->GetLocalContext()).ToLocalChecked();
            if (tryCatch.HasCaught())
            {
                Log::LogMessage message = {
                    {Log::MsgKey::Msg, "Failed to run the module"},
                    {Log::MsgKey::StackTrace, JSUtilities::GetStackTrace(m_Context->GetLocalContext(), tryCatch)}};
                LOG_ERROR(message);
                tryCatch.ReThrow();
                return V8LValue();
            }

            return result;
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

        std::string JSContextModules::GetSpecifierByModule(V8LModule inModule)
        {
            JSModuleInfoSharedPtr info = GetModuleInfoByModule(inModule, JSModuleType::kInvalid);
            if (info == nullptr)
            {
                return std::string();
            }
            return info->GetModulePath().generic_string();
        }

        V8LValue JSContextModules::GetJSONByModule(V8LModule inModule)
        {
            JSModuleInfoSharedPtr info = GetModuleInfoByModule(inModule, JSModuleType::kJSON);
            if (info == nullptr)
            {
                return V8LValue();
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

        void JSContextModules::GenerateCodeCache()
        {
            CodeCacheSharedPtr codeCache = m_Context->GetJSRuntime()->GetApp()->GetCodeCache();

            for (auto it : m_ModuleMap)
            {

                if (it.first.second != JSModuleType::kJavascript)
                {
                    continue;
                }
                if (codeCache->HasCodeCache(it.second->GetModulePath()))
                {
                    continue;
                }
                V8LModule module = it.second->GetLocalModule();
                if (module.IsEmpty())
                {
                    continue;
                }
                if (module->GetStatus() != V8Module::Status::kInstantiated)
                {
                    continue;
                    ;
                }
                V8LUnboundModScript unbound = module->GetUnboundModuleScript();
                V8ScriptCachedData *data = V8ScriptCompiler::CreateCodeCache(unbound);
                if (data->rejected || data->length == 0)
                {
                    continue;
                }
                codeCache->SetCodeCache(it.second->GetModulePath(), data);
            }
        }

        bool JSContextModules::MakeSnapshot(V8SnapshotCreatorSharedPtr inCreator, Serialization::WriteBuffer &inBuffer)
        {
            V8Isolate *isolate = m_Context->GetIsolate();
            V8IsolateScope iScope(isolate);
            V8HandleScope hScope(isolate);

            inBuffer << m_ModuleMap.size();
            for (auto &it : m_ModuleMap)
            {
                JSModuleInfo::SnapshotData data = it.second->CreateSnapshotData(inCreator);
                inBuffer << data;
                if (inBuffer.HasErrored())
                {
                    return false;
                }
            }
            return true;
        }

        bool JSContextModules::LoadSnapshotData(Serialization::ReadBuffer &inBuffer, JSContextSnapData &inContextSnapData)
        {
            // Make sure the map is clear
            m_ModuleMap.clear();
            size_t numModules;
            inBuffer >> numModules;
            if (inBuffer.HasErrored())
            {
                return false;
            }
            for (size_t idx = 0; idx < numModules; idx++)
            {
                JSModuleInfo::SnapshotData data;
                inBuffer >> data;
                if (inBuffer.HasErrored())
                {
                    return false;
                }
                inContextSnapData.m_Modules.push_back(data);
            }

            return true;
        }

        bool JSContextModules::RestoreModules(const std::vector<JSModuleInfo::SnapshotData> &inModInfoSnapData)
        {
            for (auto &data : inModInfoSnapData)
            {
                JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
                info->RestoreV8Module(data);
                auto it = m_ModuleMap.insert(std::make_pair(std::make_pair(info->GetModulePath(), info->GetAttributesInfo().m_Type), info));
                if (it.second == false)
                {
                    return false;
                }
            }
            return true;
        }

        bool JSContextModules::AddModule(const JSModuleInfoSharedPtr &inModule, std::string inFileName, JSModuleType inModuleType)
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

        JSModuleInfoSharedPtr JSContextModules::GetModuleInfoByModule(V8LModule inModule, JSModuleType inType)
        {
            V8GModule globMod(m_Context->GetIsolate(), inModule);
            for (auto it : m_ModuleMap)
            {
                if (it.second->GetLocalModule()->GetIdentityHash() == globMod.Get(m_Context->GetIsolate())->GetIdentityHash())
                {
                    if (inType == JSModuleType::kInvalid)
                    {
                        return it.second;
                    }
                    else if (it.second->GetType() == inType)
                    {
                        return it.second;
                    }
                }
            }
            return nullptr;
        }

        JSModuleInfoSharedPtr JSContextModules::BuildModuleInfo(JSModuleAttributesInfo &inAttributesInfo, const std::filesystem::path &inImportPath, const std::filesystem::path &inCurrentModPath)
        {
            // TODO:Review this for changes to use generic_string() of path for a consistent path separator
            std::filesystem::path absImportPath = inImportPath;
            Assets::AppAssetRootsSharedPtr appRoots = m_Context->GetJSRuntime()->GetApp()->GetAppRoot();
            V8Isolate *ioslate = m_Context->GetIsolate();
            JSModuleInfoSharedPtr moduleInfo = std::make_shared<JSModuleInfo>(m_Context);
            moduleInfo->SetAttributesInfo(inAttributesInfo);

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
                if (inAttributesInfo.m_Module.empty())
                {
                    absImportPath = Utils::MakeAbsolutePathToRoot(absImportPath, inCurrentModPath);
                }
                else
                {
                    std::filesystem::path modPath;
                    if (inAttributesInfo.m_Version.IsVersionString())
                    {
                        modPath = appRoots->FindModuleVersionRootPath(
                            std::filesystem::path(inAttributesInfo.m_Module) / std::filesystem::path(inAttributesInfo.m_Version.GetVersionString()));
                    }
                    else
                    {
                        modPath = appRoots->FindModuleLatestVersionRootPath(inAttributesInfo.m_Module);
                    }
                    if (modPath.empty())
                    {
                        JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::SyntaxError,
                                                  Utils::format("Failed to find attributed module: {}, ImportPath: {}", inAttributesInfo.m_Module, inImportPath));
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
                if (inAttributesInfo.m_Module.empty() == false)
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::SyntaxError,
                                              Utils::format("Can not use a module assetion in the {} root, ImportPath: {}", Assets::c_RootJS, inImportPath));
                    return nullptr;
                }
                if (inAttributesInfo.DoesExtensionMatchType(relModulePath.extension().string()) == false)
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::TypeError,
                                              Utils::format("File type doesn't match specified type {}. Importpath: {}",
                                                            JSModuleInfo::ModuleTypeToString(inAttributesInfo.m_Type), inImportPath));
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
                if (inAttributesInfo.DoesExtensionMatchType(relModulePath.extension().string()) == false)
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::TypeError,
                                              Utils::format("File type doesn't match specified type {}. Importpath: {}",
                                                            JSModuleInfo::ModuleTypeToString(inAttributesInfo.m_Type), inImportPath));
                    return nullptr;
                }

                it++;
                module = it->string();

                if (inAttributesInfo.m_Module.empty() == false && inAttributesInfo.m_Module != module)
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::SyntaxError,
                                              Utils::format("Import path was not in attributed module's path. Module:{}, ImportPath: {}",
                                                            inAttributesInfo.m_Module, inImportPath));
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
                if (inAttributesInfo.m_Module.empty() == false)
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
                if (inAttributesInfo.DoesExtensionMatchType(ext) == false)
                {
                    JSUtilities::ThrowV8Error(ioslate, JSUtilities::V8Errors::TypeError,
                                              Utils::format("File type doesn't match specified type {}. Importpath: {}",
                                                            JSModuleInfo::ModuleTypeToString(inAttributesInfo.m_Type), inImportPath));
                    return nullptr;
                }
                moduleInfo->SetPath(absImportPath);
                // files in the resources just use the filename for the module name
                moduleInfo->SetName(absImportPath.stem().string());

                return moduleInfo;
            }
            return nullptr;
        }

        V8MBLPromise JSContextModules::HostImportModuleDynamically(V8LContext inContext,
                                                                   V8LData inDefinedOptions,
                                                                   V8LValue inResourceName,
                                                                   V8LString inSpecifier,
                                                                   V8LFixedArray importoAttributes)
        {
            V8Isolate *isolate = inContext->GetIsolate();
            V8MBLPromiseResolver maybeResolver = V8PromiseResolver::New(inContext);
            V8LPromiseResolver resolver;

            if (maybeResolver.ToLocal(&resolver) == false)
            {
                return V8MBLPromise();
            }

            if (inResourceName->IsNull())
            {
                resolver->Reject(inContext, V8Exception::TypeError(V8String::NewFromUtf8Literal(isolate, "Resource name is null"))).ToChecked();
                return resolver->GetPromise();
            }

            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);
            if (jsContext == nullptr)
            {
                resolver->Reject(inContext, V8Exception::TypeError(JSUtilities::StringToV8(isolate, "Failed to get the JSContext from the v8Context"))).ToChecked();
                return resolver->GetPromise();
            }
            JSContextModulesSharedPtr jsModules = jsContext->GetJSModules();

            JSModuleAttributesInfo attributesInfo;
            attributesInfo.GetModuleAttributesInfo(jsContext, importoAttributes);
            std::filesystem::path resourceName = std::filesystem::path(JSUtilities::V8ToString(isolate, inResourceName));
            resourceName.remove_filename();
            std::filesystem::path specifier = std::filesystem::path(JSUtilities::V8ToString(isolate, inSpecifier));
            JSModuleInfoSharedPtr info = jsModules->BuildModuleInfo(attributesInfo, specifier, resourceName);

            if (attributesInfo.m_Type == JSModuleType::kInvalid)
            {
                resolver->Reject(inContext, JSUtilities::StringToV8(isolate, Utils::format("Import '{}' had an invalid type of '{}'", specifier, JSModuleInfo::ModuleTypeToString(attributesInfo.m_Type)))).ToChecked();
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
                LOG_ERROR("Failed to get the import data for importing module");
                return;
            }
            if (importData->m_Context == nullptr)
            {
                LOG_ERROR("context shared point was null for importing module");
                return;
            }
            JSContextSharedPtr jsContext = importData->m_Context;
            V8Isolate *isolate = jsContext->GetIsolate();
            if (importData->m_Resolver.IsEmpty())
            {
                LOG_ERROR("promise resolver was empty for importing module");
                return;
            }

            V8LPromiseResolver resolver = importData->m_Resolver.Get(isolate);
            JSModuleInfoSharedPtr importInfo = importData->m_ModuleInfo;
            JSContextModulesSharedPtr jsModules = jsContext->GetJSModules();

            V8HandleScope iScope(isolate);
            V8LContext context = jsContext->GetLocalContext();
            V8ContextScope cScope(context);
            V8TryCatch tryCatch(isolate);

            JSModuleInfoSharedPtr info = jsModules->GetModuleBySpecifier(importInfo->GetModulePath().generic_string());
            if (info == nullptr)
            {
                info = LoadModuleTree(jsContext, importInfo);
                if (info == nullptr)
                {
                    resolver->Reject(context, JSUtilities::StringToV8(isolate, Utils::format("Failed to load module: {}", importInfo->GetModulePath())));
                    return;
                }
            }
            V8LModule module = info->GetLocalModule();
            V8MBLValue maybeResult;
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

            V8LValue result;
            if (maybeResult.ToLocal(&result) == false)
            {
                DCHECK_TRUE(tryCatch.HasCaught());
                resolver->Reject(context, tryCatch.Exception()).ToChecked();
                return;
            }

            V8LValue moduleNamespace = module->GetModuleNamespace();
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
            V8LPromise resultPromise(result.As<V8Promise>());
            if (resultPromise->State() == V8Promise::kRejected)
            {
                resolver->Reject(context, resultPromise->Result()).ToChecked();
                return;
            }
            internal::ModuleResolutionData *data = new internal::ModuleResolutionData(jsContext, resolver, moduleNamespace);
            V8LExternal eData = V8External::New(isolate, data);
            V8LFunction callbackResolve;
            if (V8Function::New(context, JSContextModules::ResolvePromiseCallback, eData).ToLocal(&callbackResolve) == false)
            {
                resolver->Reject(context, JSUtilities::StringToV8(isolate, "Failed to create the module resolver resolve callback function")).ToChecked();
                return;
            }
            V8LFunction callbackReject;
            if (V8Function::New(context, JSContextModules::RejectPromiseCallback, eData).ToLocal(&callbackReject) == false)
            {
                resolver->Reject(context, JSUtilities::StringToV8(isolate, "Failed to create the module resolver reject callback function")).ToChecked();
                return;
            }
            resultPromise->Then(context, callbackResolve, callbackReject).ToLocalChecked();
        }

        void JSContextModules::InitializeImportMetaObject(V8LContext inContext, V8LModule inModule, V8LObject inMeta)
        {
            V8Isolate *isolate = inContext->GetIsolate();
            V8HandleScope handleScope(isolate);

            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);
            CHECK_NOT_NULL(jsContext);
            JSContextModulesSharedPtr jsModules = jsContext->GetJSModules();
            CHECK_NOT_NULL(jsModules);

            JSModuleInfoSharedPtr info = jsModules->GetModuleInfoByModule(inModule);
            CHECK_NOT_NULL(info);

            V8LString urlKey = JSUtilities::StringToV8(isolate, "url");
            V8LString url = JSUtilities::StringToV8(isolate, info->GetModulePath().generic_string());
            inMeta->CreateDataProperty(inContext, urlKey, url);
        }

        V8MBLValue JSContextModules::JSONEvalutionSteps(V8LContext inContext, V8LModule inModule)
        {
            V8LPromiseResolver resolver = V8PromiseResolver::New(inContext).ToLocalChecked();
            V8Isolate *isolate = inContext->GetIsolate();

            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);
            if (jsContext == nullptr)
            {
                LOG_ERROR("Failed to get the JSContext from the v8Context");
                resolver->Reject(inContext, v8::Undefined(isolate));
                return resolver->GetPromise();
            }
            JSContextModulesSharedPtr jsModule = jsContext->GetJSModules();
            if (jsModule == nullptr)
            {
                LOG_ERROR("Failed to get the JSContextModules from the JSCotext");
                resolver->Reject(inContext, v8::Undefined(isolate));
                return resolver->GetPromise();
            }

            V8LValue parsedJSON = jsModule->GetJSONByModule(inModule);
            if (parsedJSON.IsEmpty())
            {
                LOG_ERROR("Failed to find the module in the map");
                resolver->Reject(inContext, v8::Undefined(isolate));
                return resolver->GetPromise();
            }

            V8TryCatch tryCatch(isolate);
            v8::Maybe<bool> result = inModule->SetSyntheticModuleExport(isolate,
                                                                        JSUtilities::StringToV8(isolate,
                                                                                                "default",
                                                                                                v8::NewStringType::kInternalized),
                                                                        parsedJSON);
            if (tryCatch.HasCaught())
            {
                Log::LogMessage msg{
                    {Log::MsgKey::Msg, "Got an error trying to set the JSON module export"},
                    {Log::MsgKey::StackTrace, JSUtilities::GetStackTrace(inContext, tryCatch)}
                };
                LOG_ERROR(msg);
                resolver->Reject(inContext, v8::Undefined(isolate));
                return resolver->GetPromise();
            }

            if (result.IsNothing() || result.FromJust() == false)
            {
                LOG_ERROR("Failed to set the default export for the json module");
                resolver->Reject(inContext, v8::Undefined(isolate));
                return resolver->GetPromise();
            }

            resolver->Resolve(inContext, v8::Undefined(isolate));
            return resolver->GetPromise();
        }

        void JSContextModules::ResolvePromiseCallback(const V8FuncCallInfoValue &inInfo)
        {
            std::unique_ptr<internal::ModuleResolutionData> importData(static_cast<internal::ModuleResolutionData *>(inInfo.Data().As<V8External>()->Value()));
            V8Isolate *isolate = importData->m_Context->GetIsolate();
            V8LContext context = importData->m_Context->GetLocalContext();
            V8HandleScope hScope(isolate);

            V8LPromiseResolver resolver(importData->m_Resolver.Get(isolate));
            V8LValue moduleNamespace = importData->m_Namespace.Get(isolate);
            V8ContextScope cScope(context);

            resolver->Resolve(context, moduleNamespace).ToChecked();
        }

        void JSContextModules::RejectPromiseCallback(const V8FuncCallInfoValue &inInfo)
        {
            std::unique_ptr<internal::ModuleCallbackData> importData(static_cast<internal::ModuleCallbackData *>(inInfo.Data().As<V8External>()->Value()));
            V8Isolate *isolate = importData->m_Context->GetIsolate();
            V8LContext context = importData->m_Context->GetLocalContext();
            V8HandleScope hScope(isolate);

            V8LPromiseResolver resolver(importData->m_Resolver.Get(isolate));
            V8ContextScope cScope(context);

            DCHECK_EQ(inInfo.Length(), 1);
            resolver->Reject(context, inInfo[0]).ToChecked();
        }

        V8MBLModule JSContextModules::ResolveModuleCallback(V8LContext inContext, V8LString inSpecifier, V8LFixedArray inAttributes, V8LModule inReferrer)
        {
            V8Isolate *isolate = inContext->GetIsolate();
            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);

            JSContextModulesSharedPtr jsModule = jsContext->GetJSModules();
            std::filesystem::path filePath = std::filesystem::path(JSUtilities::V8ToString(isolate, inSpecifier));

            std::filesystem::path specifier = jsModule->GetSpecifierByModule(inReferrer);

            JSModuleAttributesInfo attributesInfo;
            attributesInfo.GetModuleAttributesInfo(jsContext, inAttributes);
            if (attributesInfo.m_Type == JSModuleType::kInvalid)
            {
                return V8MBLModule();
            }

            specifier.remove_filename();
            JSModuleInfoSharedPtr moduleInfo = jsModule->BuildModuleInfo(attributesInfo, filePath, specifier);
            if (moduleInfo == nullptr)
            {
                return V8MBLModule();
            }

            moduleInfo = jsModule->GetModuleBySpecifier(moduleInfo->GetModulePath().generic_string());
            if (moduleInfo == nullptr)
            {
                return V8MBLModule();
            }
            return moduleInfo->GetLocalModule();
        }

        JSModuleInfoSharedPtr JSContextModules::LoadModuleTree(JSContextSharedPtr inContext, const JSModuleInfoSharedPtr inModuleInfo)
        {
            V8Isolate *isolate = inContext->GetIsolate();
            V8LContext context = inContext->GetLocalContext();
            JSAppSharedPtr app = inContext->GetJSRuntime()->GetApp();
            Assets::AppAssetRootsSharedPtr appRoot = inContext->GetJSRuntime()->GetApp()->GetAppRoot();
            JSContextModulesSharedPtr jsModule = inContext->GetJSModules();

            JSModuleType moduleType = inModuleInfo->GetAttributesInfo().m_Type;
            std::filesystem::path importPath = inModuleInfo->GetModulePath();
            V8LModule module;

            if (moduleType == JSModuleType::kJavascript)
            {
                V8ScriptSourceUniquePtr source = app->GetCodeCache()->LoadScriptFile(importPath, isolate);
                if (source == nullptr)
                {
                    JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, Utils::format("Failed to load the module file: {}", importPath));
                    LOG_ERROR(Utils::format("Failed to load the module file: {}", importPath));
                    return nullptr;
                }
                V8TryCatch tryCatch(isolate);
                V8ScriptCompiler::CompileOptions options = V8ScriptCompiler::kNoCompileOptions;
                if (source->GetCachedData() != nullptr)
                {
                    options = V8ScriptCompiler::kConsumeCodeCache;
                }
                V8MBLModule maybeModule = V8ScriptCompiler::CompileModule(isolate, source.get(), options);
                if (tryCatch.HasCaught())
                {
                    tryCatch.ReThrow();
                    Log::LogMessage msg{
                        {Log::MsgKey::Msg, Utils::format("Got an error compiling module {}", importPath)},
                        {Log::MsgKey::StackTrace, JSUtilities::GetStackTrace(context, tryCatch)}
                    };
                    LOG_ERROR(msg);
                    return nullptr;
                }
                module = maybeModule.ToLocalChecked();
                inModuleInfo->SetV8Module(module);
            }
            else if (moduleType == JSModuleType::kJSON)
            {
                V8LString jsonStr;
                {
                    // TODO:: Create an asset cache for non script files
                    Assets::TextAsset file(importPath);
                    if (file.ReadAsset() == false)
                    {
                        JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, Utils::format("Failed to load the module file: {}", importPath));
                        LOG_ERROR(Utils::format("Failed to load the module file: {}", importPath));
                        return nullptr;
                    }
                    jsonStr = JSUtilities::StringToV8(isolate, file.GetContent());
                }
                V8LValue parsedJSON;
                V8TryCatch tryCatch(isolate);
                if (v8::JSON::Parse(context, jsonStr).ToLocal(&parsedJSON) == false)
                {
                    tryCatch.ReThrow();
                    LOG_ERROR(JSUtilities::GetStackTrace(context, tryCatch, importPath.string()));
                    return nullptr;
                }
                auto exportNames = v8::to_array<V8LString>({JSUtilities::StringToV8(isolate, "default")});
                module = V8Module::CreateSyntheticModule(isolate, JSUtilities::StringToV8(isolate, importPath.generic_string()), exportNames, JSContextModules::JSONEvalutionSteps);
                inModuleInfo->SetV8Module(module);
                inModuleInfo->SetV8JSON(parsedJSON);
            }
            else
            {
                LOG_ERROR(Utils::format("Uknown module type {}", JSModuleInfo::ModuleTypeToString(moduleType)));
                UNREACHABLE();
            }

            if (jsModule->AddModule(inModuleInfo, importPath.generic_string(), moduleType) == false)
            {
                LOG_ERROR(Utils::format("Failed to add module into map. File: {}", importPath));
                return nullptr;
            }

            V8LFixedArray requests = module->GetModuleRequests();
            for (int x = 0, length = requests->Length(); x < length; x++)
            {
                V8LModRequest request = requests->Get(context, x).As<V8ModRequest>();
                V8LString moduleName = request->GetSpecifier();
                V8LFixedArray v8Attributes = request->GetImportAttributes();

                JSModuleAttributesInfo attributesInfo;
                attributesInfo.GetModuleAttributesInfo(inContext, v8Attributes);
                if (attributesInfo.m_Type == JSModuleType::kInvalid)
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
                JSModuleInfoSharedPtr moduleInfo = jsModule->BuildModuleInfo(attributesInfo, requestPath, modPath);
                if (moduleInfo == nullptr)
                {
                    return nullptr;
                }
                JSModuleInfoSharedPtr cached = jsModule->GetModuleBySpecifier(moduleInfo->GetModulePath().generic_string());
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