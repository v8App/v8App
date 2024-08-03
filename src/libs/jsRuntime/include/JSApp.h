// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_APP_H_
#define _JS_APP_H_

#include <string>
#include <map>

#include "Assets/AppAssetRoots.h"
#include "Containers/NamedIndexes.h"

#include "CodeCache.h"
#include "V8Types.h"
#include "ISnapshotObject.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSApp : public std::enable_shared_from_this<JSApp>, public ISnapshotObject
        {
        public:
            JSApp(std::string inName, AppProviders inAppProviders);
            virtual ~JSApp();

            /**
             * Gets the snapshot creator for the app
             */
            IJSSnapshotCreatorSharedPtr GetSnapshotCreator();

            /**
             * Sets the app snapshot creator.
             * A nullptr will be ignored.
             */
            void SetSnapshotCreator(IJSSnapshotCreatorSharedPtr inCreator);

            /**
             * Gets the snapshot provider for the app
             */
            IJSSnapshotProviderSharedPtr GetSnapshotProvider();

            /**
             * Sets the app snapshot provider.
             * A nullptr will be ignored.
             */
            void SetSnapshotProvider(IJSSnapshotProviderSharedPtr inProvider);

            /**
             * Gets the runtime snapshot provider
             */
            IJSRuntimeProviderSharedPtr GetRuntimeProvider();
            /**
             * Sets the app runtime provider.
             * A nullptr will be ignored.
             */
            void SetRuntimeProvider(IJSRuntimeProviderSharedPtr inProvider);

            /**
             * Gets the app context provider
             */
            IJSContextProviderSharedPtr GetContextProvider();
            /**
             * Sets the app context provider.
             * A nullptr will be ignored.
             */
            void SetContextProvider(IJSContextProviderSharedPtr inProvider);

            /**
             * Initalizes stuff that can't be doe in the constrcutor like shared_from_this.
             */
            bool Initialize(std::filesystem::path inAppRoot, bool setupForSnapshot = false, AppProviders inAppPorviders = AppProviders());

            /**
             * Destorys any resources that require explicit destruction
             */
            void DisposeApp();

            /**
             * Gets the apps main runtime
             */
            JSRuntimeSharedPtr GetMainRuntime() { return m_MainRuntime; }

            /**
             * Create a new isolate that runs separate from the app's main runtime
             */
            JSRuntimeSharedPtr CreateJSRuntime(std::string inName, IdleTaskSupport inEnableIdleTasks = IdleTaskSupport::kEnabled, bool inSupportsSnapshot = false);
            /**
             * Gets the specified JSRuntime by it's name. You can fetch the main runtime as well
             * by it's name which is <app_name>-main
             */
            JSRuntimeSharedPtr GetRuntimeByName(std::string inName);

            /**
             * Disposes of one the alternative runtimes created. If you pass the main
             * runtime it'll do nothing as that runtime will last as long as the app
             */
            void DisposeRuntime(JSRuntimeSharedPtr inRuntime);
            /**
             * Disposes of the runtime with the given name. If you pass the main
             * runtimes name it will do nothing.
             */
            void DisposeRuntime(std::string inRuntimeName);

            /**
             * Gets the Code Cache object
             */
            CodeCacheSharedPtr GetCodeCache() { return m_CodeCache; };

            /**
             * Gets the app's asset roots.
             */
            Assets::AppAssetRootsSharedPtr GetAppRoot() { return m_AppAssets; }

            /**
             * Gets the apps name
             */
            std::string GetName() { return m_Name; }

            /**
             * Whether the app has neem initialized or not yet
             */
            bool IsInitialized() { return m_Initialized; }

            /**
             * Is this runtime for creatign a snapshot
             */
            bool IsSnapshotApp() { return m_IsSnapshotter; }

            JSAppSharedPtr CloneAppForSnapshotting();

            virtual bool MakeSnapshot(Serialization::WriteBuffer& inBuffer, void* inData = nullptr);
            virtual bool RestoreSnapshot(Serialization::ReadBuffer& inBufffer, void *inData = nullptr);

        protected:
            /**
             * Used by the snapshot system to clone the app by creating the correct app type.
             * Subclasses should override it to return the correct app class
             */
            virtual JSAppSharedPtr CreateSnapshotAppInstance();
            /**
             * Use by subclasses to do theiir actual init and setup
             */
            virtual bool AppInit() { return true; }

            /**
             * Create the JSRuntime
             */
            JSRuntimeSharedPtr CreateJSRuntime(std::string inName, IdleTaskSupport inEnableIdleTasks,
                                               bool setupForSnapshot, size_t inRuntimeIndex);

            /*
             * Allows subclasses to do any additional work for cloning the app for snapshotting
             */
            virtual bool CloneAppForSnapshot(JSAppSharedPtr inClonee);

            //Snoapshot serialized properties
            //****************************************/
            /** The name of the app */
            std::string m_Name;

            /** The JS rutime for the this app */
            JSRuntimeSharedPtr m_MainRuntime;

            using JSRuntimesMap = std::map<std::string, JSRuntimeSharedPtr>;

            JSRuntimesMap m_Runtimes;


            //Non Snoapshot properties
            //****************************************/
            /** The struct that holds the varois app providers */
            AppProviders m_AppProviders;

            /** Is this instance setup for snapshotting*/
            bool m_IsSnapshotter{false};

            /** The code cache for the js */
            CodeCacheSharedPtr m_CodeCache;

            /** The app assets manager */
            Assets::AppAssetRootsSharedPtr m_AppAssets;

            /** Is this instance initialized*/
            bool m_Initialized = false;

            Containers::NamedIndexes m_RuntimesSnapIndexes;

        };
    }
}

#endif //_JS_APP_H_