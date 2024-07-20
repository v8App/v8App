// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Logging/LogMacros.h"
#include "Utils/Paths.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "IJSRuntimeProvider.h"
#include "IJSSnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSApp::JSApp(std::string inName, AppProviders inAppProviders) : m_Name(inName), m_AppProviders(inAppProviders)
        {
        }

        JSApp::~JSApp()
        {
        }

        IJSSnapshotProviderSharedPtr JSApp::GetSnapshotProvider()
        {
            return m_AppProviders.m_SnapshotProvider;
        }

        void JSApp::SetSnapshotProvider(IJSSnapshotProviderSharedPtr inProvider)
        {
            // don't allow a nullptr to be set though the one in the m_AppProvider could be nullptr
            if (inProvider == nullptr)
            {
                return;
            }
            m_AppProviders.m_SnapshotProvider = inProvider;
        }

        IJSRuntimeProviderSharedPtr JSApp::GetRuntimeProvider()
        {
            return m_AppProviders.m_RuntimeProvider;
        }

        void JSApp::SetRuntimeProvider(IJSRuntimeProviderSharedPtr inProvider)
        {
            // don't allow a nullptr to be set though the one in the m_AppProvider could be nullptr
            if (inProvider == nullptr)
            {
                return;
            }
            m_AppProviders.m_RuntimeProvider = inProvider;
        }

        IJSContextProviderSharedPtr JSApp::GetContextProvider()
        {
            return m_AppProviders.m_ContextProvider;
        }

        void JSApp::SetContextProvider(IJSContextProviderSharedPtr inProvider)
        {
            // don't allow a nullptr to be set though the one in the m_AppProvider could be nullptr
            if (inProvider == nullptr)
            {
                return;
            }
            m_AppProviders.m_ContextProvider = inProvider;
        }

        bool JSApp::Initialize(std::filesystem::path inAppRoot, bool setupForSnapshot, AppProviders inAppProviders)
        {
            if (m_Initialized)
            {
                return true;
            }
            // make sure that the providers all setup and not null
            SetSnapshotProvider(inAppProviders.m_SnapshotProvider);
            SetRuntimeProvider(inAppProviders.m_RuntimeProvider);
            SetContextProvider(inAppProviders.m_ContextProvider);

            if (m_AppProviders.m_SnapshotProvider == nullptr)
            {
                Log::LogMessage msg = {{Log::MsgKey::Msg, "The snapshot provider must be set before calling Initialize"}};
                LOG_ERROR(msg);
                return false;
            }
            if (m_AppProviders.m_RuntimeProvider == nullptr)
            {
                Log::LogMessage msg = {{Log::MsgKey::Msg, "The runtime provider must be set before calling Initialize"}};
                LOG_ERROR(msg);
                return false;
            }
            if (m_AppProviders.m_ContextProvider == nullptr)
            {
                Log::LogMessage msg = {{Log::MsgKey::Msg, "The context provider must be set before calling Initialize"}};
                LOG_ERROR(msg);
                return false;
            }

            JSAppSharedPtr sharedApp = shared_from_this();

            m_AppAssets = std::make_shared<Assets::AppAssetRoots>();
            m_AppAssets->SetAppRootPath(inAppRoot);

            if (GetSnapshotProvider()->SnapshotLoaded() == false)
            {
                if (GetSnapshotProvider()->LoadSnapshotData(sharedApp) == false)
                {
                    return false;
                }
            }
            if(GetSnapshotProvider()->GetSnapshotData()->raw_size == 0)
            {
                //TODO: add logging
                return false;
            }

            m_CodeCache = std::make_shared<CodeCache>(sharedApp);

            std::string runtimeName = m_Name + "-main";
            m_MainRuntime = CreateJSRuntime(runtimeName, IdleTaskSupport::kEnabled, setupForSnapshot, 0);
            if(m_MainRuntime == nullptr)
            {
                return false;
            }
            m_IsSnapshotter = setupForSnapshot;
            m_Initialized = true;
            return true;
        }

        void JSApp::DisposeApp()
        {
            GetRuntimeProvider()->DisposeRuntime(m_MainRuntime);
            m_MainRuntime.reset();
            m_CodeCache.reset();
            m_AppAssets.reset();

            if (m_Runtimes.size())
            {
                for (auto it : m_Runtimes)
                {
                    GetRuntimeProvider()->DisposeRuntime(it.second);
                }
            }
            m_AppProviders.m_SnapshotProvider.reset();
            m_AppProviders.m_RuntimeProvider.reset();
            m_AppProviders.m_ContextProvider.reset();

            m_Runtimes.clear();
            m_IsSnapshotter = false;
            m_Initialized = false;
        }

        JSRuntimeSharedPtr JSApp::CreateJSRuntime(std::string inName, IdleTaskSupport inEnableIdleTasks)
        {
            if (inName.empty())
            {
                // TODO: log message
                return nullptr;
            }
            if (m_Runtimes.find(inName) != m_Runtimes.end())
            {
                // TODO: log message about it already being created
                return nullptr;
            }
            size_t runtimeIndex = m_RuntimesSnapIndexes.GetIndexForName(inName);
            //if we failed to find a index with that name use the main snapshot
            if(runtimeIndex == m_RuntimesSnapIndexes.GetMaxSupportedIndexes())
            {
                runtimeIndex = 0;
            }
            JSRuntimeSharedPtr runtime = CreateJSRuntime(inName, inEnableIdleTasks, false, runtimeIndex);
            if (runtime != nullptr)
            {
                auto it = m_Runtimes.insert(std::make_pair(inName, runtime));
                if (it.second)
                {
                    return runtime;
                }
            }
            return nullptr;
        }

        JSRuntimeSharedPtr JSApp::GetRuntimeByName(std::string inName)
        {
            if (GetRuntimeProvider() == nullptr)
            {
                // TODO: Log message about it
                return nullptr;
            }
            if (inName == (m_Name + "-main"))
            {
                return m_MainRuntime;
            }
            if (m_Runtimes.find(inName) != m_Runtimes.end())
            {
                return m_Runtimes[inName];
            }
            return nullptr;
        }

        void JSApp::DisposeRuntime(JSRuntimeSharedPtr inRuntime)
        {
            if (GetRuntimeProvider() == nullptr)
            {
                // TODO: Log message about it
                return;
            }
            if (inRuntime->GetName() == (m_Name + "-main"))
            {
                return;
            }
            auto it = m_Runtimes.find(inRuntime->GetName());
            if (it == m_Runtimes.end())
            {
                return;
            }
            GetRuntimeProvider()->DisposeRuntime(inRuntime);
            m_Runtimes.erase(it);
        }

        void JSApp::DisposeRuntime(std::string inRuntimeName)
        {
            if (GetRuntimeProvider() == nullptr)
            {
                // TODO: Log message about it
                return;
            }
            if (inRuntimeName == (m_Name + "-main"))
            {
                return;
            }
            auto it = m_Runtimes.find(inRuntimeName);
            if (it == m_Runtimes.end())
            {
                return;
            }
            GetRuntimeProvider()->DisposeRuntime(it->second);
            m_Runtimes.erase(it);
        }

        JSRuntimeSharedPtr JSApp::CreateJSRuntime(std::string inName, IdleTaskSupport inEnableIdleTasks,
                                 bool setupForSnapshot, size_t inRuntimeIndex)
        {
            JSRuntimeSharedPtr runtime = GetRuntimeProvider()->CreateRuntime(shared_from_this(), inName, inEnableIdleTasks, setupForSnapshot, inRuntimeIndex);

            if(runtime->Initialize() == false)
            {
                GetRuntimeProvider()->DisposeRuntime(runtime);
                return nullptr;
            }

            m_IsSnapshotter = setupForSnapshot;
            return runtime;
        }

        bool JSApp::CloneAppForSnapshot(JSAppSharedPtr inClonee)
        {
            return m_MainRuntime->CloneRuntimeForSnapshotting(inClonee->m_MainRuntime);
        }
    }
}