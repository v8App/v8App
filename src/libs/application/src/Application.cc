// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Application.h"

namespace v8App
{
    void Application::SetRooApplicationPath(std::string inRootPath)
    {
        //in release mode the root path can't be set.
#ifdef RELEASE
        return;
#endif
        m_RootAppPath = inRootPath;
    }

    std::filesystem::path Application::GetRootApplicationPath()
    {
        if(m_RootAppPath.empty())
        {
            m_RootAppPath = m_Platform->GetExecutablePath();
        }
        
        return m_RootAppPath;
    }

}