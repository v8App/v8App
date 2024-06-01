// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __BASE_ASSET_H__
#define __BASE_ASSET_H__

#include <filesystem>

namespace v8App
{
    namespace Assets
    {
        /**
         * Base class for assets
        */
        class BaseAsset
        {
        public:
            BaseAsset(std::filesystem::path inAssetPath): m_AssetPath(inAssetPath) {}
            virtual ~BaseAsset() {}

            virtual bool ReadAsset() = 0;
            virtual bool WriteAsset() = 0;

            std::filesystem::path GetPath() { return m_AssetPath; }
            bool Exists() { return std::filesystem::exists(m_AssetPath); }
            
        protected:
            std::filesystem::path m_AssetPath;
        };
    }
}

#endif //__BASE_ASSET_H__