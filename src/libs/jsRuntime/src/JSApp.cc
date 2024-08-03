// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Logging/LogMacros.h"
#include "Utils/Paths.h"
#include "Utils/Format.h"
#include "Serialization/TypeSerializer.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "IJSRuntimeProvider.h"
#include "IJSSnapshotCreator.h"
#include "IJSSnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSApp::JSApp(std::string inName, AppProviders inAppProviders)
            : m_Name(inName), m_AppProviders(inAppProviders)
        {
        }

        JSApp::~JSApp()
        {
        }

        IJSSnapshotCreatorSharedPtr JSApp::GetSnapshotCreator()
        {
            return m_AppProviders.m_SnapshotCreator;
        }

        void JSApp::SetSnapshotCreator(IJSSnapshotCreatorSharedPtr inCreator)
        {
            // don't allow a nullptr to be set though the one in the m_AppProvider could be nullptr
            if (inCreator == nullptr)
            {
                return;
            }
            m_AppProviders.m_SnapshotCreator = inCreator;
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
            SetSnapshotCreator(inAppProviders.m_SnapshotCreator);
            SetSnapshotProvider(inAppProviders.m_SnapshotProvider);
            SetRuntimeProvider(inAppProviders.m_RuntimeProvider);
            SetContextProvider(inAppProviders.m_ContextProvider);

            if (m_AppProviders.m_SnapshotProvider == nullptr)
            {
                LOG_ERROR("The snapshot provider must be set before calling Initialize");
                return false;
            }
            if (m_AppProviders.m_RuntimeProvider == nullptr)
            {
                LOG_ERROR("The runtime provider must be set before calling Initialize");
                return false;
            }
            if (m_AppProviders.m_ContextProvider == nullptr)
            {
                LOG_ERROR("The context provider must be set before calling Initialize");
                return false;
            }

            JSAppSharedPtr sharedApp = shared_from_this();
            m_IsSnapshotter = setupForSnapshot;

            m_AppAssets = std::make_shared<Assets::AppAssetRoots>();
            m_AppAssets->SetAppRootPath(inAppRoot);

            if (GetSnapshotProvider()->SnapshotLoaded() == false)
            {
                if (GetSnapshotProvider()->LoadSnapshotData(sharedApp) == false)
                {
                    return false;
                }
            }
            if (GetSnapshotProvider()->GetSnapshotData()->raw_size == 0)
            {
                // TODO: add logging
                return false;
            }

            m_CodeCache = std::make_shared<CodeCache>(sharedApp);

            std::string runtimeName = m_Name + "-main";
            // the main runtime always supports snapshotting
            m_MainRuntime = CreateJSRuntime(runtimeName, IdleTaskSupport::kEnabled, true, 0);
            if (m_MainRuntime == nullptr)
            {
                return false;
            }
            if (AppInit() == false)
            {
                return false;
            }
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
            m_AppProviders.m_SnapshotCreator.reset();
            m_AppProviders.m_SnapshotProvider.reset();
            m_AppProviders.m_RuntimeProvider.reset();
            m_AppProviders.m_ContextProvider.reset();

            m_Runtimes.clear();
            m_IsSnapshotter = false;
            m_Initialized = false;
        }

        JSAppSharedPtr JSApp::CloneAppForSnapshotting()
        {
            JSAppSharedPtr newApp = CreateSnapshotAppInstance();
            if( newApp->CloneAppForSnapshot(shared_from_this())== false)
            {
                LOG_ERROR(Utils::format("Faield to clone app {} for snapshotting", m_Name));
                return nullptr;
            }
            return newApp;
        }

        bool JSApp::MakeSnapshot(Serialization::WriteBuffer &inBuffer, void *inData)
        {
            JSAppSharedPtr snapApp;
            bool createdApp = false;

            if (m_IsSnapshotter == false)
            {
                LOG_ERROR(Utils::format("The app '{}' is not a snapshot app", m_Name));
                return false;
            }

            if (GetSnapshotCreator() == nullptr)
            {
                LOG_ERROR("The snapshot creator must be set before before calling MakeSnapshot");
                return false;
            }

            inBuffer << m_Name;

            // main runtie is written first
            if (m_MainRuntime->MakeSnapshot(inBuffer) == false)
            {
                return false;
            }

            // a snapshot app only has runtimes to snapshot
            // they should be created for snapshotif
            // CreateJSRuntime is used with an app that is snapshottable
            // or a non snapshottable app was clone for snapshotting
            inBuffer << m_Runtimes.size();
            for (auto [name, runtime] : m_Runtimes)
            {
                if (runtime->MakeSnapshot(inBuffer) == false)
                {
                    return false;
                }
            }
        }

        bool JSApp::RestoreSnapshot(Serialization::ReadBuffer &inBufffer, void *inData)
        {
            return false;
        }

        JSAppSharedPtr JSApp::CreateSnapshotAppInstance()
        {
            return std::make_shared<JSApp>(m_Name + "-snapshotter", m_AppProviders);
        }

        JSRuntimeSharedPtr JSApp::CreateJSRuntime(std::string inName, IdleTaskSupport inEnableIdleTasks, bool inSupportsSnapshot)
        {
            if (inName.empty())
            {
                LOG_ERROR("Passed runtime name was empty for CreateJSRuntime");
                return nullptr;
            }
            if (m_Runtimes.find(inName) != m_Runtimes.end())
            {
                LOG_ERROR(Utils::format("A runtime with name '{}' already exists", inName));
                return nullptr;
            }
            size_t runtimeIndex = m_RuntimesSnapIndexes.GetIndexForName(inName);
            // if we failed to find a index with that name use the main snapshot
            if (runtimeIndex == m_RuntimesSnapIndexes.GetMaxSupportedIndexes())
            {
                runtimeIndex = 0;
            }
            JSRuntimeSharedPtr runtime = CreateJSRuntime(inName, inEnableIdleTasks, inSupportsSnapshot, runtimeIndex);
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
                LOG_ERROR("Runtime provder was null wheb calling DisposeRuntime");
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
                LOG_ERROR("Runtime provder was null wheb calling DisposeRuntime");
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
                                                  bool inSupportsSnapshotting, size_t inRuntimeIndex)
        {
            // depending on whether the app is a snapshotter depends pn if the runtime is inited for it
            JSRuntimeSharedPtr runtime = GetRuntimeProvider()->CreateRuntime(shared_from_this(), inName, inEnableIdleTasks, m_IsSnapshotter, inRuntimeIndex);

            if (runtime->Initialize(inSupportsSnapshotting) == false)
            {
                GetRuntimeProvider()->DisposeRuntime(runtime);
                return nullptr;
            }

            return runtime;
        }

        bool JSApp::CloneAppForSnapshot(JSAppSharedPtr inClonee)
        {
            JSAppSharedPtr app = shared_from_this();

            m_AppProviders = inClonee->m_AppProviders;
            m_IsSnapshotter = true;
            m_AppAssets = std::make_shared<Assets::AppAssetRoots>();
            m_AppAssets->SetAppRootPath(inClonee->m_AppAssets->GetAppRoot());
            // TODO: Porbably need to clone it as well or perhaps the cache will be reworked
            //  as it's initial design was before learning somethings and the Asset stuff
            m_CodeCache = std::make_shared<CodeCache>(app);

            JSRuntimeSharedPtr runtime = inClonee->m_MainRuntime->CloneRuntimeForSnapshotting(app);
            if (runtime == nullptr)
            {
                return false;
            }
            m_MainRuntime = runtime;

            for (auto [name, cloneRuntime] : inClonee->m_Runtimes)
            {
                if (runtime->CanBeSnapshotted() == false)
                {
                    continue;
                }
                runtime = cloneRuntime->CloneRuntimeForSnapshotting(app);
                if (runtime == nullptr)
                {
                    return false;
                }
                auto it = m_Runtimes.insert(std::make_pair(runtime->GetName(), runtime));
                if (it.second == false)
                {
                    return false;
                }
            }
            m_Initialized = true;
            return true;
        }
    }
}