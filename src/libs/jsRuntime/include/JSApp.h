// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_APP_H_
#define _JS_APP_H_

#include <string>
#include <map>

#include "Assets/AppAssetRoots.h"

#include "CodeCache.h"
#include "JSContext.h"
#include "V8Types.h"
#include "V8SnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSApp : public std::enable_shared_from_this<JSApp>
        {
        public:
            JSApp(std::string inName, V8SnapshotProviderSharedPtr inSnapshotProvider = V8SnapshotProviderSharedPtr());
            virtual ~JSApp();

            /**
             * Initalizes stuff that can't be doe in the constrcutor like shared_from_this.
             */
            bool Initialize(std::filesystem::path inAppRoot, bool setupForSnapshot = false, JSContextCreationHelperSharedPtr inContextCreator = nullptr);

            /**
             * Use by subclasses to do theiir actual init and setup
             */
            virtual bool AppInit();

            /**
             * Destorys up any resources that require explicit destruction
             */
            void DisposeApp();

            /**
             * Gets the apps runtime
             */
            JSRuntimeSharedPtr GetJSRuntime();

            /**
             * Create a context from the runtime using the specified namespace.
             * If no namespace is provided then a v8 context with just the global namespace
             */
            JSContextSharedPtr CreateJSContext(std::string inName, std::filesystem::path inEntryPoint, std::string inNamespace = "",
                                             std::filesystem::path inSnapEntryPoint = "", bool inSupportsSnapshot = true,
                                             SnapshotMethod inSnapMethod = SnapshotMethod::kNamespaceOnly);

            /**
             * Gets a context from the JSRuntime.
             */
            JSContextSharedPtr GetJSContextByName(std::string inName);

            /**
             * Gets the Code Cache object
             */
            CodeCacheSharedPtr GetCodeCache();

            /**
             * Gets the app's asset roots.
             */
            Assets::AppAssetRootsSharedPtr GetAppRoots();

            /**
             * Gets the apps name
             */
            std::string GetName();

            /**
             * Whether the app has neem initialized or not yet
             */
            bool IsInitialized() { return m_Initialized; }

            /**
             * Sets the app js script entry point.
             */
            bool SetEntryPointScript(std::filesystem::path inEntryPpint);

            /**
             * Gets the app entry point script.
             */
            std::filesystem::path GetEntryPointScript() { return m_AppEntryPoint; }

            /**
             * Creates a version fo the app for snapshoting. If the app is already a setup for snapshotting it just returns itse;f
             */
            JSAppSharedPtr CreateSnapshotApp();

            /**
             * Gets the snapshot creator for the app.
             */
            V8SnapshotCreatorSharedPtr GetSnapshotCreator();

            /**
             * Is this runtime for creatign a snapshot
             */
            bool IsSnapshotCreator() { return m_IsSnapshotter; }

            void SetSnapshotProvider(V8SnapshotProviderSharedPtr inProvider)
            {
                if (inProvider != nullptr)
                {
                    m_SnapshotProvider = inProvider;
                }
            }

            V8SnapshotProviderSharedPtr GetSnapshotProvider()
            {
                return m_SnapshotProvider;
            }

        protected:
            /**
             * Create the JSRuntime
             */
            bool CreateJSRuntime(std::string inName, JSContextCreationHelperSharedPtr inContextCreator, bool setupForSnapshot);

            /** The name of the app */
            std::string m_Name;

            /** The class that provides the snapshot data*/
            V8SnapshotProviderSharedPtr m_SnapshotProvider;

            /** Is this instance setup for snapshotting*/
            bool m_IsSnapshotter = false;

            /** The code cache for the js */
            CodeCacheSharedPtr m_CodeCache;

            /** The app assets manager */
            Assets::AppAssetRootsSharedPtr m_AppAssets;

            /** The entry point script for the app */
            std::filesystem::path m_AppEntryPoint;

            /** Is this instance initialized*/
            bool m_Initialized = false;

            /** The JS rutime for the this app */
            JSRuntimeSharedPtr m_JSRuntime;
        };
    }
}

#endif //_JS_APP_H_