// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Logging/LogMacros.h"

#include "V8Types.h"
#include "JSUtilities.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"
#include "CppBridge/V8CppObject.h"

namespace v8App
{
    using Utils::CallbackWrapper;

    namespace JSRuntime
    {
        namespace CppBridge
        {
            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(V8Isolate *inIsolate, const char *inTypeName)
                : m_Isolate(inIsolate), m_TypeName(inTypeName)
            {
            }

            V8ObjectTemplateBuilder::V8ObjectTemplateBuilder(const V8ObjectTemplateBuilder &inTemplate) = default;

            V8ObjectTemplateBuilder::~V8ObjectTemplateBuilder() = default;

            V8LFuncTpl V8ObjectTemplateBuilder::Build()
            {
                V8LFuncTpl templ = m_Constructor;
                m_ProtoTemplate.Clear();
                m_Constructor.Clear();
                return templ;
            }

            V8ObjectTemplateBuilder &V8ObjectTemplateBuilder::SetConstructorInternal(const std::string &inName, V8LFuncTpl inConstructor)
            {
                // assert if the template was passed in externally since we don't know what was setup before it was passed
                CHECK_TRUE(m_Constructor.IsEmpty());

                m_Constructor = inConstructor;
                // recreate the object template with the constrcutor
                m_ProtoTemplate.Clear();
                m_ProtoTemplate = inConstructor->PrototypeTemplate();
                m_ProtoTemplate->SetInternalFieldCount((int)V8CppObjDataIntField::MaxInternalFields);

                 m_Constructor->SetClassName(JSUtilities::StringToV8(m_Isolate, inName));

                return *this;
            }

            V8ObjectTemplateBuilder &V8ObjectTemplateBuilder::SetValueMethodInternal(const std::string &inName, V8LData inValue)
            {
                //make sure the constructor has been set
                CHECK_FALSE(m_Constructor.IsEmpty());
                m_ProtoTemplate->Set(JSUtilities::CreateSymbol(m_Isolate, inName), inValue);
                return *this;
            }

            V8ObjectTemplateBuilder &V8ObjectTemplateBuilder::SetPropertyInternal(const std::string &inName, V8LFuncTpl inGetter,
                                                                                  V8LFuncTpl inSetter)
            {
                //make sure the constructor has been set
                CHECK_FALSE(m_Constructor.IsEmpty());
                m_ProtoTemplate->SetAccessorProperty(JSUtilities::CreateSymbol(m_Isolate, inName), inGetter, inSetter);
                return *this;
            }
        }
    }
}