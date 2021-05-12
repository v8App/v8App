// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "CppBridge/V8ObjectTemplateBuilder.h"

namespace v8App
{
    using Utils::CallbackWrapper;

    namespace JSRuntime
    {
        namespace CppBridge
        {
            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(IsolateWeakPtr inIsolate, int inNumInternalFields)
                : V8ObjectTemplateBuilder(inIsolate, inNumInternalFields, nullptr)
            {
            }

            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(IsolateWeakPtr inIsolate, int inNumInternalFields, const char *inTypeNmae)
                : m_Isolate(inIsolate), m_TypeName(inTypeNmae)
            {
                CHECK_EQ(false, m_Isolate.expired());

                m_Template = v8::ObjectTemplate::New(m_Isolate.lock().get());
                m_Template->SetInternalFieldCount(inNumInternalFields);
            }

            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(IsolateWeakPtr inIsolate, int inNumInternalFields, const char *inTypeNmae, v8::Local<v8::ObjectTemplate> inTemplate) : m_Isolate(inIsolate), m_TypeName(inTypeNmae)
            {
                CHECK_EQ(false, m_Isolate.expired());

                m_Template = v8::ObjectTemplate::New(m_Isolate.lock().get());
                m_Template->SetInternalFieldCount(inNumInternalFields);
            }

            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(const V8ObjectTemplateBuilder &inTemplate) = default;

            V8ObjectTemplateBuilder::~V8ObjectTemplateBuilder() = default;

            v8::Local<v8::ObjectTemplate> V8ObjectTemplateBuilder::Build()
            {
                v8::Local<v8::ObjectTemplate> templ = m_Template;
                templ.Clear();
                return templ;
            }

            V8ObjectTemplateBuilder &V8ObjectTemplateBuilder::SetValueMethodInternal(const std::string &inName, v8::Local<v8::Data> inValue)
            {
                CHECK_EQ(false, m_Isolate.expired());
                m_Template->Set(CreateSymbol(m_Isolate.lock().get(), inName), inValue);
                return *this;
            }

            V8ObjectTemplateBuilder &V8ObjectTemplateBuilder::SetPropertyInternal(const std::string &inName, v8::Local<v8::FunctionTemplate> inGetter,
                                                                                  v8::Local<v8::FunctionTemplate> inSetter)
            {
                CHECK_EQ(false, m_Isolate.expired());
                m_Template->SetAccessorProperty(CreateSymbol(m_Isolate.lock().get(), inName), inGetter, inSetter);
                return *this;
            }
        }
    }
}