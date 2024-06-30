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
            if (m_Loaded == false)
            {
                if (LoadDataFile(inApp, inSnapshotPath))
                {
                    m_Loaded = true;
                }
            }

            return m_Loaded;
        }

        bool V8SnapshotProvider::LoadDataFile(JSAppSharedPtr inApp, std::filesystem::path inSnapshotPath)
        {
            if (inSnapshotPath.empty() == false)
            {
                m_SnapshotPath = inSnapshotPath;
            }
            if (m_SnapshotPath.empty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "A path needs to be specified at construction or passed to LoadSnapshotData"}};
                LOG_ERROR(msg);
                return false;
            }
            std::filesystem::path absPath = inApp->GetAppRoots()->MakeAbsolutePathToAppRoot(m_SnapshotPath);
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

            std::ifstream snapData(absPath, std::ios_base::binary | std::ios_base::ate);
            if (snapData.is_open() == false || snapData.fail())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to open the snapshot file {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }
            int dataLength = snapData.tellg();
            snapData.seekg(0, std::ios::beg);
            std::unique_ptr<char> buf = std::unique_ptr<char>(new char[dataLength]);
            snapData.read(buf.get(), dataLength);
            if (snapData.fail())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to read the snapshot file {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }
            m_StartupData = V8StartupData{buf.release(), dataLength};
            return true;
        }

        v8::SerializeInternalFieldsCallback V8SnapshotProvider::GetInternalSerializerCallaback()
        {
            return v8::SerializeInternalFieldsCallback(&V8SnapshotProvider::SerializeInternalField_Internal, this);
        }

        v8::SerializeContextDataCallback V8SnapshotProvider::GetContextSerializerCallback()
        {
            return v8::SerializeContextDataCallback(&V8SnapshotProvider::SerializeContextInternalField_Internal, this);
        }

        v8::DeserializeInternalFieldsCallback V8SnapshotProvider::GetInternalDeserializerCallback()
        {
            return v8::DeserializeInternalFieldsCallback(V8SnapshotProvider::DeserializeInternalField_Internal, this);
        }

        v8::DeserializeContextDataCallback V8SnapshotProvider::GetContextDeserializerCallaback()
        {
            return v8::DeserializeContextDataCallback(V8SnapshotProvider::DeserializeContextInternalField_Internal, this);
        }

        V8StartupData V8SnapshotProvider::SerializeInternalField_Internal(V8LObject inHolder, int inIndex, void *inData)
        {
            V8SnapshotProvider *provider = static_cast<V8SnapshotProvider *>(inData);
            if (provider != nullptr)
            {
                provider->SerializeInternalField(inHolder, inIndex);
            }
            return {nullptr, 0};
        }

        V8StartupData V8SnapshotProvider::SerializeContextInternalField_Internal(V8LContext inHolder, int inIndex, void *inData)
        {
            V8SnapshotProvider *provider = static_cast<V8SnapshotProvider *>(inData);
            if (provider != nullptr)
            {
                provider->SerializeContextInternalField(inHolder, inIndex);
            }
            return {nullptr, 0};
        }

        void V8SnapshotProvider::DeserializeInternalField_Internal(V8LObject inHolder, int inIndex, V8StartupData inPayload, void *inData)
        {
            V8SnapshotProvider *provider = static_cast<V8SnapshotProvider *>(inData);
            if (provider != nullptr)
            {
                provider->DeserializeInternalField(inHolder, inIndex, inPayload);
            }
        }

        void V8SnapshotProvider::DeserializeContextInternalField_Internal(V8LContext inHolder, int inIndex, V8StartupData inPayload, void *inData)
        {
            V8SnapshotProvider *provider = static_cast<V8SnapshotProvider *>(inData);
            if (provider != nullptr)
            {
                provider->DeserializeContextInternalField(inHolder, inIndex, inPayload);
            }
        }
    }
}
