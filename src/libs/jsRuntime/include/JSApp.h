// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_APP_H_
#define _JS_APP_H_

#include <string>
#include <map>

#include "Assets/AppAssetRoots.h"
#include "Containers/NamedIndexes.h"
#include "Utils/VersionString.h"

#include "CodeCache.h"
#include "V8Types.h"
#include "ISnapshotObject.h"
#include "JSAppCreatorRegistry.h"
#include "JSAppSnapData.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSApp : public std::enable_shared_from_this<JSApp>, public ISnapshotObject
        {
        public:
            inline static const char *kDefaulV8tRuntimeName{"v8-default"};
            inline static size_t kDefaultV8RutimeIndex = 0;

            JSApp();
            virtual ~JSApp();

            /**
             * Initalizes stuff that can't be doe in the constrcutor like shared_from_this.
             */
            virtual bool Initialize(std::string inAppName, std::filesystem::path inAppRoot, AppProviders inAppPorviders, bool setupForSnapshot = false);

            /**
             * Initializes an app from loaded snapshot data
             */
            virtual bool ResotreInitialize(std::filesystem::path inAppRoot, AppProviders inAppProviders);

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
            JSRuntimeSharedPtr CreateJSRuntimeFromName(std::string inRuntimeName, std::string inSnapRuntimeName = kDefaulV8tRuntimeName, JSRuntimeSnapshotAttributes inSnapAttribute = JSRuntimeSnapshotAttributes::NotSnapshottable,
                                                       IdleTaskSupport inEnableIdleTasks = IdleTaskSupport::kEnabled);
            JSRuntimeSharedPtr CreateJSRuntimeFromIndex(std::string inRuntimeName, size_t inSanpRuntimeIndex = kDefaultV8RutimeIndex, JSRuntimeSnapshotAttributes inSnapAttribute = JSRuntimeSnapshotAttributes::NotSnapshottable,
                                                        IdleTaskSupport inEnableIdleTasks = IdleTaskSupport::kEnabled);
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
            bool IsInitialized() { return m_AppState >= JSAppStates::Initialized; }
            /**
             * Is the app in a running state
             */
            bool IsRunning() { return m_AppState == JSAppStates::Running; }

            /**
             * Is this runtime for creatign a snapshot
             */
            bool IsSnapshotApp() { return m_IsSnapshotter; }

            /**
             * Clones this app for snapshotting
             */
            JSAppSharedPtr CloneAppForSnapshotting();

            /**
             * Creates a snapshot of the data writing it to the passed buffer
             */
            virtual bool MakeSnapshot(Serialization::WriteBuffer &inBuffer, void *inData = nullptr);
            /**
             * Loads the snap data from the file into the Snap Data object
             */
            virtual JSAppSnapDataSharedPtr LoadSnapshotData(Serialization::ReadBuffer &inBufffer);
            /**
             * Restores the app from the snap data
             */
            virtual bool RestoreSnapshot(JSAppSnapDataSharedPtr inSnapdata);
            /**
             * Subclasses should override and return their snap data object if they have otehr data they
             * need snapshotted
             */
            virtual JSAppSnapDataSharedPtr CreateSnapData() { return std::make_shared<JSAppSnapData>(); }

            /**
             * Gets the app version
             */
            Utils::VersionString &GetAppVersion() { return m_AppVersion; }
            /**
             * Sets teh app version by string
             */
            void SetAppVersion(std::string inVersion);
            /**
             * Sets the version by Version class
             */
            void SetAppVersion(const Utils::VersionString &inVersion);
            /**
             * Function to create the class from serialized type string
             */
            static JSAppSharedPtr AppCreator() { return std::make_shared<JSApp>(); }
            /**
             * Returns the class's type strign used when serializing
             */
            virtual std::string GetClassType() { return s_ClassType; }

        protected:
            /**
             * base class static subclasses should use the macro to overide
             */
            inline static const std::string s_ClassType{"JSApp"};

            /**
             * Use by subclasses to do theiir actual init and setup
             */
            virtual bool AppInit() { return true; }

            /**
             * Sets and checks the app providers for init and restore init
             */
            bool SetAndCheckAppProviders(AppProviders &inAppProviders);

            /**
             * Create the JSRuntime
             */
            JSRuntimeSharedPtr CreateJSRuntime(std::string inName, IdleTaskSupport inEnableIdleTasks,
                                               size_t inRuntimeIndex, JSRuntimeSnapshotAttributes inSnapAttrib);

            /*
             * Allows subclasses to do any additional work for cloning the app for snapshotting
             */
            virtual bool CloneAppForSnapshot(JSAppSharedPtr inClonee);

            /**
             * Name of the app
             */
            std::string m_Name;

            /**
             * The JS rutime for the this app
             */
            JSRuntimeSharedPtr m_MainRuntime;

            /**
             * Other runtimes created that are not main
             */
            using JSRuntimesMap = std::map<std::string, JSRuntimeSharedPtr>;
            JSRuntimesMap m_Runtimes;

            /**
             * The version of the app
             */
            Utils::VersionString m_AppVersion{"0.0.0"};

            /** Non Snoapshot properties
             * ***************************************/
            /** 
             * Holds the order of runtime creation so that when snapshotting
             * we can reverse the order do to the v8::SanpshotCreator automaticlly
             * entering the isolate when it's created and we need to reverse the
             * destory order so they exit in reverse or we get an error
             */
            std::vector<JSRuntimeSharedPtr> m_DestroyOrder;

            /** The struct that holds the varois app providers */
            AppProviders m_AppProviders;

            /** Is this instance setup for snapshotting*/
            bool m_IsSnapshotter{false};

            /** The code cache for the js */
            CodeCacheSharedPtr m_CodeCache;

            /** The app assets manager */
            Assets::AppAssetRootsSharedPtr m_AppAssets;

            /** Is this instance initialized*/
            JSAppStates m_AppState{JSAppStates::Uninitialized};
        };

        REGISTER_JSAPP_CREATOR(JSApp, JSApp::AppCreator)
    }
}

/**
 * Macros to define the static class type variable and getter for subclasses
 */
#define REGISTER_JSAPP_CLASS_TYPE(classType)                      \
    inline static const std::string s_CT_##classType{#classType}; \
    virtual std::string GetClassType() override { return s_CT_##classType; }

#endif //_JS_APP_H_