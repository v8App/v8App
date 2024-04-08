// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Logging/LogMacros.h"
#include "Utils/Paths.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "JSContext.h"

namespace v8App
{
    namespace JSRuntime
    {
        v8::StartupData V8BaseSnapshotProvider::s_V8StartupData{nullptr, 0};
        std::filesystem::path V8BaseSnapshotProvider::s_SnapshotPath;

        JSApp::JSApp(std::string inName, JSSnapshotProviderSharedPtr inSnapshotProvider) : m_Name(inName), m_SnapshotProvider(inSnapshotProvider)
        {
            DCHECK_NOT_NULL(m_SnapshotProvider.get());
        }

        JSApp::~JSApp()
        {
            if (m_JSRuntime != nullptr)
            {
                m_JSRuntime->DisposeRuntime();
            }
        }

        bool JSApp::InitializeRuntime(std::filesystem::path inAppRoot, std::filesystem::path inSnapshotFile, bool setupForSnapshot)
        {
            JSAppSharedPtr sharedApp = shared_from_this();
            if (m_Initialized)
            {
                return true;
            }
            if (m_SnapshotProvider == nullptr)
            {
                return false;
            }
            m_AppAssets = std::make_shared<Assets::AppAssetRoots>();
            m_AppAssets->SetAppRootPath(inAppRoot);
            m_CodeCache = std::make_shared<CodeCache>(sharedApp);

            if (inSnapshotFile != "")
            {
                if (m_SnapshotProvider->LoadSnapshotData(inSnapshotFile, sharedApp) == false)
                {
                    return false;
                }
            }

            std::string runtimeName = m_Name + "-runtime";
            if (CreateJSRuntime(runtimeName, setupForSnapshot, nullptr) == false)
            {
                return false;
            }
            m_Initialized = true;
            return true;
        }

        bool JSApp::AppInit()
        {
            JSContextSharedPtr appContext = m_JSRuntime->CreateContext("main");
            if (appContext == nullptr)
            {
                return false;
            }
        }

        void JSApp::DisposeApp()
        {
            m_Creator.reset();
            m_JSRuntime->DisposeRuntime();
            m_JSRuntime.reset();
            m_CodeCache.reset();
            m_SnapshotProvider.reset();
            m_AppAssets.reset();
            m_IsSnapshotter = false;
            m_Initialized = false;
        }

        JSRuntimeSharedPtr JSApp::GetJSRuntime()
        {
            return m_JSRuntime;
        }

        JSContextSharedPtr JSApp::CreateJSContext(std::string inName)
        {
            if (m_JSRuntime != nullptr)
            {
                return m_JSRuntime->CreateContext(inName);
            }
            return nullptr;
        }

        JSContextSharedPtr JSApp::GetJSContextByName(std::string inName)
        {
            if (m_JSRuntime == nullptr)
            {
                return nullptr;
            }
            return m_JSRuntime->GetContextByName(inName);
        }

        CodeCacheSharedPtr JSApp::GetCodeCache()
        {
            return m_CodeCache;
        }

        Assets::AppAssetRootsSharedPtr JSApp::GetAppRoots()
        {
            return m_AppAssets;
        }

        std::string JSApp::GetName()
        {
            return m_Name;
        }

        bool JSApp::SetEntryPointScript(std::filesystem::path inEntryPpint)
        {
            std::filesystem::path absPath = m_AppAssets->MakeAbsolutePathToAppRoot(inEntryPpint);
            if (absPath.empty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Passed entry point script may have escaped the app root. File:{}", inEntryPpint)}};
                LOG_ERROR(msg);
                return false;
            }
            if (std::filesystem::exists(absPath) == false)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Passed entry point script doesn't exist {}", inEntryPpint)}};
                LOG_ERROR(msg);
                return false;
            }
            m_AppEntryPoint = absPath;
            return true;
        }

        JSAppSharedPtr JSApp::CreateSnapshotApp()
        {
            if (m_IsSnapshotter)
            {
                return shared_from_this();
            }
            JSAppSharedPtr snapApp = std::make_shared<JSApp>(m_Name + "-snapshotter", m_SnapshotProvider);
            if (snapApp->InitializeRuntime(m_AppAssets->GetAppRoot(), "", true) == false)
            {
                return nullptr;
            }
            snapApp->m_IsSnapshotter = true;
            return snapApp;
        }

        v8::SnapshotCreator *JSApp::GetSnapshotCreator()
        {
            return m_Creator.get();
        }

        bool JSApp::CreateJSRuntime(std::string inName, bool setupForSnapshot, const intptr_t *inExternalReferences)
        {
            const v8::StartupData *data = m_SnapshotProvider->GetSnapshotData();
            if (data == nullptr)
            {
                return false;
            }
            if (setupForSnapshot)
            {
                m_JSRuntime = JSRuntime::CreateJSRuntimeForSnapshot(shared_from_this(), IdleTasksSupport::kIdleTasksEnabled, inName);
                m_Creator = std::make_unique<v8::SnapshotCreator>(m_JSRuntime->GetIsolate(), inExternalReferences, data, false);
            }
            else
            {
                m_JSRuntime = JSRuntime::CreateJSRuntime(shared_from_this(), IdleTasksSupport::kIdleTasksEnabled, inName, data, inExternalReferences);
            }
            if (m_JSRuntime == nullptr)
            {
                return false;
            }
            m_JSRuntime->Initialize();
            JSContextCreationHelperUniquePtr helper = std::make_unique<JSContextCreator>();
            m_JSRuntime->SetContextCreationHelper(std::move(helper));
            return true;
        }

        bool V8BaseSnapshotProvider::LoadSnapshotData(std::filesystem::path inSnaopshotFile, JSAppSharedPtr inApp)
        {
            std::filesystem::path absPath = inApp->GetAppRoots()->MakeAbsolutePathToAppRoot(inSnaopshotFile);
            if (absPath.empty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Passed snapshot data may have escaped the app root. File:{}", inSnaopshotFile)}};
                LOG_ERROR(msg);
                return false;
            }
            if (std::filesystem::exists(absPath) == false)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Passed snapshot data doesn't exist {}", inSnaopshotFile)}};
                LOG_ERROR(msg);
                return false;
            }
            // if it's already loaded and the same path then skip loading
            if (s_SnapshotPath == absPath && s_V8StartupData.raw_size != 0)
            {
                return true;
            }
            s_SnapshotPath = absPath;

            std::ifstream snapData(absPath, std::ios_base::binary | std::ios_base::ate);
            if (snapData.is_open() == false || snapData.fail())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to open the snapshot file {}", inSnaopshotFile)}};
                LOG_ERROR(msg);
                return false;
            }
            int dataLength = snapData.tellg();
            snapData.seekg(0, std::ios::beg);
            std::unique_ptr<char> buf = std::unique_ptr<char>(new char[dataLength]);
            snapData.read(buf.get(), dataLength);
            if (snapData.fail())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to read the snapshot file {}", inSnaopshotFile)}};
                LOG_ERROR(msg);
                return false;
            }
            s_V8StartupData = v8::StartupData{buf.release(), dataLength};
            return true;
        }

        const v8::StartupData *V8BaseSnapshotProvider::GetSnapshotData()
        {
            return &s_V8StartupData;
        }
    }
}