// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Logging/LogMacros.h"
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
            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(v8::Isolate *inIsolate)
                : V8ObjectTemplateBuilder(inIsolate, nullptr)
            {
            }

            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(v8::Isolate *inIsolate, const char *inTypeName)
                : V8ObjectTemplateBuilder(inIsolate, inTypeName, v8::ObjectTemplate::New(inIsolate), true)
            {
            }

            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(v8::Isolate *inIsolate, const char *inTypeNmae, v8::Local<v8::ObjectTemplate> inTemplate, bool inConstructorAllowed)
                : m_Isolate(inIsolate), m_Template(inTemplate), m_ConstructorAllowed(inConstructorAllowed), m_TypeName(inTypeNmae)
            {
                m_Template->SetInternalFieldCount(kMaxReservedInternalFields);
            }

            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(const V8ObjectTemplateBuilder &inTemplate) = default;

            V8ObjectTemplateBuilder::~V8ObjectTemplateBuilder() = default;

            v8::Local<v8::ObjectTemplate> V8ObjectTemplateBuilder::Build()
            {
                v8::Local<v8::ObjectTemplate> templ = m_Template;
                m_Template.Clear();
                return templ;
            }

            V8ObjectTemplateBuilder &V8ObjectTemplateBuilder::SetConstructorInternal(const std::string &inName, v8::Local<v8::FunctionTemplate> inConstructor)
            {
                //assert if the template was passed in externally since we don't know what was setup before it was passed
                CHECK_EQ(m_ConstructorAllowed, true);
                //recreate the object template with the constrcutor
                m_Template.Clear();
                m_Template = inConstructor->PrototypeTemplate();
                m_Template->SetInternalFieldCount(kMaxReservedInternalFields);

                inConstructor->SetClassName(JSUtilities::StringToV8(m_Isolate, inName));
                v8::Local<v8::Context> context = m_Isolate->GetCurrentContext();
                v8::Local<v8::Object> global = context->Global();
                CHECK_FALSE(global.IsEmpty());

                global->Set(context, JSUtilities::StringToV8(m_Isolate, inName), inConstructor->GetFunction(context).ToLocalChecked());
                
                return *this;
            }

            V8ObjectTemplateBuilder &V8ObjectTemplateBuilder::SetValueMethodInternal(const std::string &inName, v8::Local<v8::Data> inValue)
            {
                //once we start set stuff don't allow the constrcutor to be added
                m_ConstructorAllowed = false;
                m_Template->Set(JSUtilities::CreateSymbol(m_Isolate, inName), inValue);
                return *this;
            }

            V8ObjectTemplateBuilder &V8ObjectTemplateBuilder::SetPropertyInternal(const std::string &inName, v8::Local<v8::FunctionTemplate> inGetter,
                                                                                  v8::Local<v8::FunctionTemplate> inSetter)
            {
                //once we start set stuff don't allow the constrcutor to be added
                m_ConstructorAllowed = false;
                m_Template->SetAccessorProperty(JSUtilities::CreateSymbol(m_Isolate, inName), inGetter, inSetter);
                return *this;
            }
        }
    }
}