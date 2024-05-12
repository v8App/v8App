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
        const v8::StartupData *JSSnapshotProvider::GetSnapshotData()
        {
            return &m_V8StartupData;
        }

        std::filesystem::path JSSnapshotProvider::GetSnapshotPath()
        {
            return m_SnapshotPath;
        }

        bool JSSnapshotProvider::LoadSnapshotData(JSAppSharedPtr inApp, std::filesystem::path inSnapshotPath)
        {
            if (m_Loaded)
            {
                return true;
            }
            if (inSnapshotPath.empty() == false)
            {
                m_SnapshotPath = inSnapshotPath;
            }
            if (m_SnapshotPath.empty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "A path needs to be specified at construction or passed to LoadSnapshotData"}};
                LOG_ERROR(msg);
                return false;
            }
            std::filesystem::path absPath = inApp->GetAppRoots()->MakeAbsolutePathToAppRoot(m_SnapshotPath);
            if (absPath.empty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Specified snapshot path may have escaped the app root. File:{}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }
            if (std::filesystem::exists(absPath) == false)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Passed snapshot path doesn't exist {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }

            m_SnapshotPath = absPath;

            std::ifstream snapData(absPath, std::ios_base::binary | std::ios_base::ate);
            if (snapData.is_open() == false || snapData.fail())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to open the snapshot file {}", m_SnapshotPath)}};
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
                    {Log::MsgKey::Msg, Utils::format("Failed to read the snapshot file {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }
            m_V8StartupData = v8::StartupData{buf.release(), dataLength};
            m_Loaded = true;
            return true;
        }

        JSApp::JSApp(std::string inName, JSSnapshotProviderSharedPtr inSnapshotProvider) : m_Name(inName), m_SnapshotProvider(inSnapshotProvider)
        {
        }

        JSApp::~JSApp()
        {
            if (m_JSRuntime != nullptr)
            {
                m_JSRuntime->DisposeRuntime();
            }
        }

        bool JSApp::InitializeApp(std::filesystem::path inAppRoot, bool setupForSnapshot)
        {
            if (m_Initialized)
            {
                return true;
            }
            if (m_SnapshotProvider == nullptr)
            {
                Log::LogMessage msg = {{Log::MsgKey::Msg, "The snapshot provider must be set before calling InitializeApp"}};
                LOG_ERROR(msg);
                return false;
            }

            JSAppSharedPtr sharedApp = shared_from_this();
            if (m_SnapshotProvider->SnapshotLoaded() == false)
            {
                if (m_SnapshotProvider->LoadSnapshotData(sharedApp) == false)
                {
                    return false;
                }
            }

            m_AppAssets = std::make_shared<Assets::AppAssetRoots>();
            m_AppAssets->SetAppRootPath(inAppRoot);
            m_CodeCache = std::make_shared<CodeCache>(sharedApp);

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
            if (m_IsSnapshotter == false)
            {
                JSContextSharedPtr appContext = m_JSRuntime->CreateContext("main");
                if (appContext == nullptr)
                {
                    return false;
                }
            }
            return true;
        }

        void JSApp::DisposeApp()
        {
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
            if (snapApp->InitializeApp(m_AppAssets->GetAppRoot(), true) == false)
            {
                return nullptr;
            }
            snapApp->m_IsSnapshotter = true;
            return snapApp;
        }

        V8SnapshotCreatorSharedPtr JSApp::GetSnapshotCreator()
        {
            if(m_JSRuntime != nullptr)
            {
            return m_JSRuntime->GetSnapshotCreator();
            }
            return nullptr;
        }

        bool JSApp::CreateJSRuntime(std::string inName, bool setupForSnapshot, const intptr_t *inExternalReferences)
        {
            const v8::StartupData *data = m_SnapshotProvider->GetSnapshotData();
            if (data->raw_size == 0)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "Snapshot data seems to be empty"}};
                LOG_ERROR(msg);
                return false;
            }
            m_JSRuntime = JSRuntime::CreateJSRuntime(shared_from_this(), IdleTasksSupport::kIdleTasksEnabled, inName, data, inExternalReferences, setupForSnapshot);
            if (setupForSnapshot)
            {
                m_IsSnapshotter = true;
            }
            else
            {
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
    }
}