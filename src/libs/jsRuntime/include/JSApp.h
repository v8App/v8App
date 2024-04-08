// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_APP_H_
#define _JS_APP_H_

#include <string>
#include <map>

#include "Assets/AppAssetRoots.h"

#include "CodeCache.h"
#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Class used to load and provider the snapshot data. So test and the main app can handle things differently
         */
        class JSSnapshotProvider
        {
        public:
            virtual ~JSSnapshotProvider() = default;
            virtual bool LoadSnapshotData(std::filesystem::path, JSAppSharedPtr inApp) = 0;
            virtual const v8::StartupData *GetSnapshotData() = 0;
        };

        using JSSnapshotProviderSharedPtr = std::shared_ptr<JSSnapshotProvider>;

        class JSApp : public std::enable_shared_from_this<JSApp>
        {
        public:
            JSApp(std::string inName, JSSnapshotProviderSharedPtr inSnapshotProvider);
            virtual ~JSApp();

            /**
             * Initalizes stuff that can't be doe in the constrcutor like shared_from_this.
             */
            bool InitializeRuntime(std::filesystem::path inAppRoot, std::filesystem::path inSnapshotFile, bool setupForSnapshot = false);

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
             * Create a context from the runtim
             */
            JSContextSharedPtr CreateJSContext(std::string inName);

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
             * Create the snapshot for the app saving in te specified file.
             */
            bool CreateSnapshot(std::filesystem::path inSnapshotFile);

           /**
             * Gets the snapshot creator for the app.
             */
            v8::SnapshotCreator* GetSnapshotCreator();

            /**
             * Is this runtime for creatign a snapshot
             */
            bool IsSnapshotCreator() { return m_IsSnapshotter; }

        protected:
            /**
             * Create the JSRuntime
             */
            bool CreateJSRuntime(std::string inName, bool setupForSnapshot, const intptr_t *inExternalReferences);


            /** The name of the app */
            std::string m_Name;

            /** The class that provides the snapshot data*/
            JSSnapshotProviderSharedPtr m_SnapshotProvider;

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
            V8SnapshotCreatorUniquePtr m_Creator;
        };

        class V8BaseSnapshotProvider : public JSSnapshotProvider
        {
        public:
            virtual bool LoadSnapshotData(std::filesystem::path, JSAppSharedPtr inApp) override;
            virtual const v8::StartupData *GetSnapshotData() override;

        private:
            /** Snapshot data */
            static v8::StartupData s_V8StartupData;
            /** Path loaded from in case a new one is loaded */
            static std::filesystem::path s_SnapshotPath;
        };
    }
}

#endif //_JS_APP_H_