// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "JSModuleInfo.h"
#include "JSContext.h"
namespace v8App
{
    namespace JSRuntime
    {
        JSModuleInfo::JSModuleInfo(JSContextSharedPtr inContext) : m_Context(inContext)
        {
        }
        JSModuleInfo::~JSModuleInfo()
        {
            m_Module.Reset();
            m_Context.reset();
        }

        void JSModuleInfo::SetPath(std::filesystem::path inPath)
        {
            m_Path = inPath;
        }

        std::filesystem::path JSModuleInfo::GetModulePath() const
        {
            return m_Path;
        }

        void JSModuleInfo::SetName(std::string inName)
        {
            m_ModuleName = inName;
        }

        std::string JSModuleInfo::GetName() const
        {
            return m_ModuleName;
        }

        void JSModuleInfo::SetVersion(std::string inVersion)
        {
            m_Version = inVersion;
        }

        Utils::VersionString JSModuleInfo::GetVersion() const
        {
            return m_Version;
        }

        void JSModuleInfo::SetV8Module(V8LModule &inModule)
        {
            m_Module.Reset(m_Context->GetIsolate(), inModule);
        }

        V8LModule JSModuleInfo::GetLocalModule()
        {
            return m_Module.Get(m_Context->GetIsolate());
        }

        void JSModuleInfo::SetV8JSON(V8LValue &inValue)
        {
            m_JSON.Reset(m_Context->GetIsolate(), inValue);
        }

        V8LValue JSModuleInfo::GetLocalJSON()
        {
            return m_JSON.Get(m_Context->GetIsolate());
        }

        void JSModuleInfo::SetUnboundScript(V8LUnboundModScript &inScript)
        {
            m_UnboundScript.Reset(m_Context->GetIsolate(), inScript);
        }

        V8LUnboundModScript JSModuleInfo::GetUnboundScript()
        {
            return m_UnboundScript.Get(m_Context->GetIsolate());
        }
        void JSModuleInfo::ClearUnboundScript()
        {
            m_UnboundScript.Reset();
        }

        void JSModuleInfo::SetAttributesInfo(AttributesInfo &inInfo)
        {
            m_AttributesInfo = inInfo;
        }

        const JSModuleInfo::AttributesInfo &JSModuleInfo::GetAttributesInfo() const
        {
            return m_AttributesInfo;
        }
    }
}