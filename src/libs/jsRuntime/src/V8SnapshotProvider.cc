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
        bool V8SnapshotProvider::LoadSnapshotData(std::filesystem::path inSnapshotPath)
        {
            if (m_Loaded && inSnapshotPath == m_SnapshotPath)
            {
                return true;
            }
            if (inSnapshotPath.empty() == false)
            {
                m_SnapshotPath = inSnapshotPath;
            }
     
            if (m_SnapshotPath.empty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "A path needs to be passed to LoadSnapshotData"}};
                LOG_ERROR(msg);
                return false;
            }

            if (std::filesystem::exists(m_SnapshotPath) == false)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Passed snapshot path doesn't exist {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }

            m_SnapshotPath = inSnapshotPath;
            m_Loaded = false;
            m_AssetFile.SetPath(m_SnapshotPath);

            if (m_AssetFile.ReadAsset() == false)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to read the snapshot file {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }

            const Assets::BinaryByteVector &data = m_AssetFile.GetContent();

            m_StartupData = V8StartupData{reinterpret_cast<const char *>(data.data()), (int)data.size()};
            m_Loaded = true;

            return m_Loaded;
        }
    }
}
