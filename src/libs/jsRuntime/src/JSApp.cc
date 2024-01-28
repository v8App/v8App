// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "JSApp.h"
#include "JSRuntime.h"
#include "JSContext.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSApp::JSApp(std::string inName) : m_Name(inName)
        {
            m_AppAssets = std::make_shared<Assets::AppAssetRoots>();
        }

        JSApp::~JSApp()
        {
            for (auto runtime : m_JSRuntimes)
            {
                runtime.second->DisposeRuntime();
            }
            m_JSRuntimes.clear();
        }

        void JSApp::Initialize()
        {
            if (m_CodeCache == nullptr)
            {
                m_CodeCache = std::make_shared<CodeCache>(shared_from_this());
            }
        }

        void JSApp::DisposeApp()
        {
            if (m_CodeCache != nullptr)
            {
                m_CodeCache.reset();
            }
        }

        JSRuntimeSharedPtr JSApp::CreateJSRuntime(std::string inName)
        {
            if (m_JSRuntimes.find(inName) != m_JSRuntimes.end())
            {
                return nullptr;
            }
            return InternalCreateJSRutnime(inName);
        }

        JSRuntimeSharedPtr JSApp::CreateJSRuntimeOrGet(std::string inName)
        {
            if (m_JSRuntimes.find(inName) != m_JSRuntimes.end())
            {
                return m_JSRuntimes[inName];
            }
            return InternalCreateJSRutnime(inName);
        }

        JSRuntimeSharedPtr JSApp::GetJSRuntimeByName(std::string inName)
        {
            auto it = m_JSRuntimes.find(inName);
            if (it == m_JSRuntimes.end())
            {
                return nullptr;
            }
            return it->second;
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

        JSRuntimeSharedPtr JSApp::InternalCreateJSRutnime(std::string inName)
        {
            JSRuntimeSharedPtr runtime = JSRuntime::CreateJSRuntime(shared_from_this(), IdleTasksSupport::kIdleTasksEnabled, inName);
            JSContextCreationHelperUniquePtr helper = std::make_unique<JSContextCreator>();
            runtime->SetContextCreationHelper(std::move(helper));
            auto it = m_JSRuntimes.insert(std::make_pair(inName, runtime));
            if (it.second)
            {
                return runtime;
            }
            return nullptr;
        }

        void JSApp::DisposeRuntime(JSRuntime *inRuntime)
        {
            if(inRuntime == nullptr)
            {
                return;
            }
            if(m_JSRuntimes.find(inRuntime->GetName()) != m_JSRuntimes.end())
            {
                m_JSRuntimes.erase(inRuntime->GetName());
            }
            inRuntime->DisposeRuntime();
        }

    }
}