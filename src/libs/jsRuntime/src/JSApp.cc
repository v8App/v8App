// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>
#include <ranges>

#include "Logging/LogMacros.h"
#include "Utils/Paths.h"
#include "Utils/Format.h"
#include "Serialization/TypeSerializer.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "IJSRuntimeProvider.h"
#include "IJSSnapshotCreator.h"
#include "IJSSnapshotProvider.h"
#include "JSRuntimeSnapData.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSApp::JSApp()
        {
        }

        JSApp::~JSApp()
        {
        }

        bool JSApp::Initialize(std::string inAppName, std::filesystem::path inAppRoot, AppProviders inAppProviders, bool setupForSnapshot)
        {
            if (IsInitialized())
            {
                return true;
            }
            if (m_AppState == JSAppStates::Restored)
            {
                LOG_ERROR("Initalize can not be called on a restored app. Use RestoreInitialize.");
                return false;
            }

            m_Name = inAppName;
            if (SetAndCheckAppProviders(inAppProviders) == false)
            {
                return false;
            }

            JSAppSharedPtr sharedApp = shared_from_this();
            m_IsSnapshotter = setupForSnapshot;

            m_AppAssets = std::make_shared<Assets::AppAssetRoots>();
            m_AppAssets->SetAppRootPath(inAppRoot);

            if (GetSnapshotProvider()->SnapshotLoaded() == false && GetSnapshotProvider()->GetSnapshotPath().empty() == false)
            {
                if (GetSnapshotProvider()->LoadSnapshotData() == false)
                {
                    return false;
                }
            }
            if (GetSnapshotProvider()->GetSnapshotData(0)->raw_size == 0)
            {
                LOG_ERROR("Snapshot provider says data is loaded but default snapshot size is 0");
                return false;
            }

            m_CodeCache = std::make_shared<CodeCache>(sharedApp);

            std::string runtimeName = m_Name + "-main";
            // the main runtime always supports snapshotting
            m_MainRuntime = CreateJSRuntime(runtimeName, IdleTaskSupport::kEnabled, 0, JSRuntimeSnapshotAttributes::SnapshotAndRestore);

            if (m_MainRuntime == nullptr)
            {
                return false;
            }
            if (AppInit() == false)
            {
                return false;
            }
            m_AppState = JSAppStates::Initialized;
            return true;
        }

        bool JSApp::ResotreInitialize(std::filesystem::path inAppRoot, AppProviders inAppProviders)
        {
            if (IsInitialized())
            {
                return true;
            }
            if (m_AppState == JSAppStates::Uninitialized)
            {
                LOG_ERROR("RestoreInitalize can not be called on a non restored app. Use Initialize.");
                return false;
            }

            if (SetAndCheckAppProviders(inAppProviders) == false)
            {
                return false;
            }

            // TODO: add code to do the rest of the init

            m_AppState = JSAppStates::Initialized;
            return true;
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

        void JSApp::DisposeApp()
        {
            if (m_DestroyOrder.size())
            {
                // Destory hte runtimes in the reverse order of creation
                for (auto it : std::ranges::views::reverse(m_DestroyOrder))
                {
                    GetRuntimeProvider()->DisposeRuntime(it);
                }
            }
            m_DestroyOrder.clear();
            m_Runtimes.clear();

            GetRuntimeProvider()->DisposeRuntime(m_MainRuntime);
            m_MainRuntime.reset();
            m_CodeCache.reset();
            m_AppAssets.reset();

            m_AppProviders.m_SnapshotCreator.reset();
            m_AppProviders.m_SnapshotProvider.reset();
            m_AppProviders.m_RuntimeProvider.reset();
            m_AppProviders.m_ContextProvider.reset();

            m_IsSnapshotter = false;
            m_AppState = JSAppStates::Uninitialized;
        }

        JSAppSharedPtr JSApp::CloneAppForSnapshotting()
        {
            JSAppSharedPtr newApp = JSAppCreatorRegistry::CreateApp(GetClassType());
            if (newApp->CloneAppForSnapshot(shared_from_this()) == false)
            {
                LOG_ERROR(Utils::format("Faield to clone app {} for snapshotting", m_Name));
                return nullptr;
            }
            return newApp;
        }

        bool JSApp::MakeSnapshot(Serialization::WriteBuffer &inBuffer, void *inData)
        {
            JSAppSharedPtr snapApp;

            if (m_IsSnapshotter == false)
            {
                LOG_ERROR(Utils::format("The app '{}' is not a snapshot app", m_Name));
                return false;
            }

            inBuffer << m_AppVersion;
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
            return true;
        }

        JSAppSnapDataSharedPtr JSApp::LoadSnapshotData(Serialization::ReadBuffer &inBuffer)
        {
            JSAppSnapDataSharedPtr snapData = CreateSnapData();

            inBuffer >> snapData->m_AppVersion;
            if (inBuffer.HasErrored())
            {
                LOG_ERROR("Buffer read failed on reading app version");
                return {};
            }
            inBuffer >> snapData->m_Name;
            if (inBuffer.HasErrored())
            {
                LOG_ERROR("Buffer read failed on reading app name");
                return {};
            }

            JSRuntime runtimeLoader;
            JSRuntimeSnapDataSharedPtr runtime = runtimeLoader.LoadSnapshotData(inBuffer);
            if (runtime == nullptr)
            {
                LOG_ERROR("Failed to load the main runtime data");
                return {};
            }
            snapData->m_RuntimesSnapData.push_back(runtime);

            runtime = nullptr;
            size_t numRuntimes;
            inBuffer >> numRuntimes;
            if (inBuffer.HasErrored())
            {
                LOG_ERROR("Buffer read failed on reading number of runtimes");
                return {};
            }

            for (size_t idx = 0; idx < numRuntimes; idx++)
            {
                runtime = runtimeLoader.LoadSnapshotData(inBuffer);
                if (runtime == nullptr)
                {
                    LOG_ERROR(Utils::format("Failed to load the index {} runtime data", idx));
                    return {};
                }
                snapData->m_RuntimesSnapData.push_back(runtime);
                snapData->m_RuntimesSnapIndexes.AddNamedIndex(idx, runtime->m_RuntimeName);
                runtime = nullptr;
            }

            return snapData;
        }

        bool JSApp::RestoreSnapshot(JSAppSnapDataSharedPtr inSnapData)
        {
            return false;
        }

        JSRuntimeSharedPtr JSApp::CreateJSRuntimeFromName(std::string inRuntimeName, std::string inSnapRuntimeName,
                                                          JSRuntimeSnapshotAttributes inSnapAttribute,
                                                          IdleTaskSupport inEnableIdleTasks)
        {

            if (inSnapRuntimeName.empty())
            {
                LOG_ERROR("Passed snap runtime name was empty for CreateJSRuntime");
                return nullptr;
            }

            size_t runtimeIndex = GetSnapshotProvider()->GetIndexForRuntimeName(inSnapRuntimeName);
            return CreateJSRuntimeFromIndex(inRuntimeName, runtimeIndex, inSnapAttribute, inEnableIdleTasks);
        }

        JSRuntimeSharedPtr JSApp::CreateJSRuntimeFromIndex(std::string inRuntimeName, size_t inSnapRuntimeIndex,
                                                           JSRuntimeSnapshotAttributes inSnapAttribute,
                                                           IdleTaskSupport inEnableIdleTasks)
        {
            if (inRuntimeName.empty())
            {
                LOG_ERROR("Passed runtime name was empty for CreateJSRuntime");
                return nullptr;
            }

            if (m_Runtimes.find(inRuntimeName) != m_Runtimes.end())
            {
                LOG_ERROR(Utils::format("A runtime with name '{}' already exists", inRuntimeName));
                return nullptr;
            }

            JSRuntimeSharedPtr runtime = CreateJSRuntime(inRuntimeName, inEnableIdleTasks, inSnapRuntimeIndex, inSnapAttribute);
            if (runtime != nullptr)
            {
                auto it = m_Runtimes.insert(std::make_pair(inRuntimeName, runtime));
                if (it.second)
                {
                    m_DestroyOrder.push_back(runtime);
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
            if (inRuntime != nullptr)
            {
                DisposeRuntime(inRuntime->GetName());
            }
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
            auto vIt = std::find(m_DestroyOrder.begin(), m_DestroyOrder.end(), it->second);
             if (vIt != m_DestroyOrder.end())
            {
                m_DestroyOrder.erase(vIt);
            }
            m_Runtimes.erase(it);
        }

        JSRuntimeSharedPtr JSApp::CreateJSRuntime(std::string inName, IdleTaskSupport inEnableIdleTasks,
                                                  size_t inRuntimeIndex, JSRuntimeSnapshotAttributes inSnapAttrib)
        {
            JSRuntimeSharedPtr runtime = GetRuntimeProvider()->CreateRuntime();

            if (runtime->Initialize(shared_from_this(), inName, inRuntimeIndex, inSnapAttrib, m_IsSnapshotter, inEnableIdleTasks) == false)
            {
                GetRuntimeProvider()->DisposeRuntime(runtime);
                return nullptr;
            }

            return runtime;
        }

        bool JSApp::SetAndCheckAppProviders(AppProviders &inAppProviders)
        {
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
            return true;
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
            m_AppState = JSAppStates::Initialized;
            return true;
        }
    }
}