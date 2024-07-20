// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Logging/LogMacros.h"
#include "Utils/Paths.h"
#include "Utils/Format.h"

#include "JSApp.h"
#include "V8SnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        bool V8SnapshotProvider::LoadSnapshotData(JSAppSharedPtr inApp, std::filesystem::path inSnapshotPath)
        {
            if(m_Loaded && inSnapshotPath == m_SnapshotPath)
            {
                return true;
            }
            m_Loaded = false;
            if (inSnapshotPath.empty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "A path needs to be passed to LoadSnapshotData"}};
                LOG_ERROR(msg);
                return false;
            }
            m_SnapshotPath = inSnapshotPath;
            std::filesystem::path absPath = inApp->GetAppRoot()->MakeAbsolutePathToAppRoot(m_SnapshotPath);
            if (absPath.empty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Specified snapshot path may have escaped the app root. File:{}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }
            if (std::filesystem::exists(absPath) == false)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Passed snapshot path doesn't exist {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }

            m_SnapshotPath = absPath;
            m_AssetFile.SetPath(absPath);

            std::ifstream snapData(absPath, std::ios_base::binary | std::ios_base::ate);
            if (m_AssetFile.ReadAsset() == false)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to read the snapshot file {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }

            const Assets::BinaryByteVector& data = m_AssetFile.GetContent();

            m_StartupData = V8StartupData{reinterpret_cast<const char*>(data.data()), (int)data.size()};
            m_Loaded = true;

            return m_Loaded;
        }
    }
}
