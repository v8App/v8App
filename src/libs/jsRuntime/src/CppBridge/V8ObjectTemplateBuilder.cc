// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Logging/LogMacros.h"

#include "V8Types.h"
#include "JSUtilities.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"
#include "CppBridge/V8NativeObject.h"

namespace v8App
{
    using Utils::CallbackWrapper;

    namespace JSRuntime
    {
        namespace CppBridge
        {
            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(v8::Isolate *inIsolate, v8::Local<v8::Object> inGlobal, const char *inTypeName)
                : m_Isolate(inIsolate), m_Global(inGlobal), m_TypeName(inTypeName)
            {
            }

            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(const V8ObjectTemplateBuilder &inTemplate) = default;

            V8ObjectTemplateBuilder::~V8ObjectTemplateBuilder() = default;

            v8::Local<v8::ObjectTemplate> V8ObjectTemplateBuilder::Build()
            {
                v8::Local<v8::ObjectTemplate> templ = m_Template;
                m_Template.Clear();
                m_Global.Clear();
                return templ;
            }

            V8ObjectTemplateBuilder &V8ObjectTemplateBuilder::SetConstructorInternal(const std::string &inName, v8::Local<v8::FunctionTemplate> inConstructor, v8::Local<v8::Context> inContext)
            {
                // assert if the template was passed in externally since we don't know what was setup before it was passed
                CHECK_FALSE(m_ConstructorAllowed);
                // recreate the object template with the constrcutor
                m_Template.Clear();
                m_Template = inConstructor->PrototypeTemplate();
                m_Template->SetInternalFieldCount((int)V8CppObjDataIntField::MaxInternalFields);

                 inConstructor->SetClassName(JSUtilities::StringToV8(m_Isolate, inName));

                m_Global->Set(inContext, JSUtilities::StringToV8(m_Isolate, inName), inConstructor->GetFunction(inContext).ToLocalChecked());

                return *this;
            }

            V8ObjectTemplateBuilder &V8ObjectTemplateBuilder::SetValueMethodInternal(const std::string &inName, v8::Local<v8::Data> inValue)
            {
                // once we start set stuff don't allow the constrcutor to be added
                m_ConstructorAllowed = false;
                m_Template->Set(JSUtilities::CreateSymbol(m_Isolate, inName), inValue);
                return *this;
            }

            V8ObjectTemplateBuilder &V8ObjectTemplateBuilder::SetPropertyInternal(const std::string &inName, v8::Local<v8::FunctionTemplate> inGetter,
                                                                                  v8::Local<v8::FunctionTemplate> inSetter)
            {
                // once we start set stuff don't allow the constrcutor to be added
                m_ConstructorAllowed = false;
                m_Template->SetAccessorProperty(JSUtilities::CreateSymbol(m_Isolate, inName), inGetter, inSetter);
                return *this;
            }
        }
    }
}