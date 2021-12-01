// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "JSContext.h"
#include "V8TestFixture.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSContextTest = V8TestFixture;

        TEST_F(JSContextTest, Test)
        {
            v8::HandleScope handleScopr(m_Isolate);

            JSContext jsContext(m_Isolate);
            EXPECT_EQ(m_Isolate, jsContext.GetIsolate());

            v8::Local<v8::Context> context = jsContext.GetContext();
            EXPECT_TRUE(context.IsEmpty());
            JSModules* jsModules = jsContext.GetModules().lock().get();
            EXPECT_NE(nullptr, jsModules);

            EXPECT_TRUE(jsContext.InitializeContext());

            context = jsContext.GetContext();
            EXPECT_FALSE(context.IsEmpty());
            //test that calling init a second time just returns

            EXPECT_TRUE(jsContext.InitializeContext());
            v8::Local<v8::Context> context2 = jsContext.GetContext();
            EXPECT_TRUE(context == context2);

            //test that we can get the JSContext prointer back
            JSContext* jsContext2 = JSContext::GetJSContext(context);
            EXPECT_EQ(&jsContext, jsContext2);


            //test move copy constrcutor
            JSContext jsContext3(std::move(jsContext));
            //orginal object
            EXPECT_EQ(nullptr, jsContext.GetModules().lock().get());
            EXPECT_TRUE(jsContext.GetContext().IsEmpty());
            EXPECT_EQ(nullptr, jsContext.GetIsolate());
            EXPECT_NE(&jsContext, JSContext::GetJSContext(context));

            //new object
            EXPECT_EQ(jsModules, jsContext3.GetModules().lock().get());
            EXPECT_FALSE(jsContext3.GetContext().IsEmpty());
            EXPECT_EQ(m_Isolate, jsContext3.GetIsolate());
            EXPECT_EQ(&jsContext3, JSContext::GetJSContext(context));


            //test move assignemnt operator
            jsContext = std::move(jsContext3);
            //new object
            EXPECT_EQ(nullptr, jsContext3.GetModules().lock().get());
            EXPECT_TRUE(jsContext3.GetContext().IsEmpty());
            EXPECT_EQ(nullptr, jsContext3.GetIsolate());
            EXPECT_NE(&jsContext3, JSContext::GetJSContext(context));

            //original object
            EXPECT_EQ(jsModules, jsContext.GetModules().lock().get());
            EXPECT_FALSE(jsContext.GetContext().IsEmpty());
            EXPECT_EQ(m_Isolate, jsContext.GetIsolate());
            EXPECT_EQ(&jsContext, JSContext::GetJSContext(context));


            //tets dispose
            jsContext.DisposeContext();
            jsContext2 = static_cast<JSContext*>(context->GetAlignedPointerFromEmbedderData(0));
            EXPECT_EQ(nullptr, jsContext2);
            EXPECT_EQ(nullptr, jsContext.GetIsolate());
            EXPECT_EQ(true, jsContext.GetContext().IsEmpty());
            EXPECT_EQ(nullptr, jsContext.GetModules().lock().get());
        }
    }
}
