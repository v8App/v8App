// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <Windows.h>

#include <string>
#include <memory>
#include <iostream>
#include <filesystem>

#include "platforms/windows/WinPlatform.h"

namespace v8App {
    std::filesystem::path WinPlatform::GetExecutablePath()
    {
        //we limt the max path to be returned to 2k
        const DWORD MAX_PATH_LIMIT = 1024*2;
        
        DWORD pathBufferSize = MAX_PATH;
        std::unique_ptr<WCHAR> pathBuffer{new WCHAR[pathBufferSize]};

        while(MAX_PATH_LIMIT> pathBufferSize) 
        {
            const auto bytesWritten = GetModuleFileNameW(NULL, pathBuffer.get(), pathBufferSize);
            const auto lastError = GetLastError();

            if(bytesWritten == 0)
            {
                //TODO add a log
                return std::filesystem::path();
            }

            if(lastError == ERROR_INSUFFICIENT_BUFFER)
            {
                pathBufferSize *= 2;
                pathBuffer.reset(new WCHAR[pathBufferSize]);
                continue;
            }

            //finished getting the path
            break;
        }

        pathBufferSize = GetFullPathNameW(pathBuffer.get(), 0, NULL, NULL);
        if(pathBufferSize == 0)
        {
            //TODO: add log 
            return std::filesystem::path();
        }

        std::unique_ptr<WCHAR> fullNameBuffer{new WCHAR[pathBufferSize]};
        DWORD result = GetFullPathNameW(pathBuffer.get(), pathBufferSize, fullNameBuffer.get(), NULL);
        if(result == 0)
        {
            //TODO: add log
            return std::filesystem::path();
        }

        pathBufferSize = GetLongPathNameW(fullNameBuffer.get(), 0, NULL);
        if(pathBufferSize == 0)
        {
            //TODO: add log 
            return std::filesystem::path();
        }

        std::unique_ptr<WCHAR> longNameBuffer{new WCHAR[pathBufferSize]};
        result = GetFullPathNameW(fullNameBuffer.get(), pathBufferSize, longNameBuffer.get(), NULL);
        if(result == 0)
        {
            //TODO: add log
            return std::filesystem::path();
        }

        return std::filesystem::path(longNameBuffer.get());
    }

}