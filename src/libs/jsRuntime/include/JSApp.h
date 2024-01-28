// Copyright 2020 The v8App Authors. All rights reserved.
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
        class JSApp : public std::enable_shared_from_this<JSApp>
        {
        public:
            JSApp(std::string inName);
            virtual ~JSApp();

            /**
             * Initalizes stuff that can't be doe in the constrcutor like shared_from_this.
            */
           void Initialize();

           /**
            * Destorys up any resources that require explicit destruction
           */
          void DisposeApp();

            /**
             * Create a JSRuntime witht he given name. If a runtime with the name already exists
             * a empty shared pointer is returned
             */
            JSRuntimeSharedPtr CreateJSRuntime(std::string inName);
            /**
             * Creates a JSRuntime with the given name. If a runtime with the given name already
             * exists it will return it instead of creating a new one.
             */
            JSRuntimeSharedPtr CreateJSRuntimeOrGet(std::string inName);
            /**
             * Gets a JSRuntime with the given name if it exists or returns an empty shared poiter.
             */
            JSRuntimeSharedPtr GetJSRuntimeByName(std::string inName);

            /**
             * Dsiposes of a runtime and removes it from tracking.
            */
            void DisposeRuntime(JSRuntime* inRuntime);

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

        protected:
            /**
             * Create the JSRuntime
             */
            JSRuntimeSharedPtr InternalCreateJSRutnime(std::string inName);

            std::string m_Name;
            CodeCacheSharedPtr m_CodeCache;
            Assets::AppAssetRootsSharedPtr m_AppAssets;

            std::map<std::string, JSRuntimeSharedPtr> m_JSRuntimes;
        };
    }
}

#endif //_JS_APP_H_