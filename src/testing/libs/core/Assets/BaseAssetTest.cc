// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "test_main.h"

#include "Assets/BaseAsset.h"

namespace v8App
{
    namespace Assets
    {
        class TestBaseAsset : public BaseAsset
        {
            public:
            TestBaseAsset(std::filesystem::path inAssetPath) : BaseAsset(inAssetPath){}

            virtual bool ReadAsset() override {return true; }
            virtual bool WriteAsset() override {return true; }
        };

        TEST(BaseAssetTest, Constrcutor)
        {
            std::filesystem::path tmp = s_TestDir;
            tmp /= std::filesystem::path("baseAsset.txt");

            TestBaseAsset asset(tmp);

            EXPECT_EQ(asset.GetPath().string(), tmp.string());
        }
        
        TEST(BaseAssetTest, Exists)
        {
            std::filesystem::path tmp = s_TestDir;
            tmp /= std::filesystem::path("baseAsset.txt");
            //make sure it's removed
            std::filesystem::remove(tmp);

            TestBaseAsset asset(tmp);

            EXPECT_FALSE(asset.Exists());

            std::ofstream file(tmp);
            file.flush();

            EXPECT_TRUE(asset.Exists());
        }
    } // namespace Assets
    
    
} // namespace v8App
