// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_OBJECT_TEMPLATE_BUILDER_H__
#define __V8_OBJECT_TEMPLATE_BUILDER_H__

#include <functional>

#include "v8.h"
#include "Logging/LogMacros.h"
#include "Utils/CallbackWrapper.h"
#include "JSRuntime.h"
#include "V8Arguments.h"
#include "V8FunctionTemplate.h"

namespace v8App
{
    using Utils::CallbackWrapper;

    namespace JSRuntime
    {
        namespace CppBridge
        {
            class V8ObjectTemplateBuilder
            {
            public:
                explicit V8ObjectTemplateBuilder(v8::Isolate* inIsolate);
                V8ObjectTemplateBuilder(v8::Isolate* inIsolate, const char *inTypeName);
                V8ObjectTemplateBuilder(v8::Isolate* inIsolate, const char *inTypeName, v8::Local<v8::ObjectTemplate> inTemplate, bool inConstructorAllows = false);

                V8ObjectTemplateBuilder(const V8ObjectTemplateBuilder &inTemplate);
                ~V8ObjectTemplateBuilder();

                // Use the classes typename for the v8 symbol name
                template <typename T>
                V8ObjectTemplateBuilder &SetConstuctor(const T &inCallback)
                {
                    return SetConstuctor(m_TypeName, inCallback);
                }

                //allows specifying the symbol name for setting the v8 symbol for the function 
                template <typename T>
                V8ObjectTemplateBuilder &SetConstuctor(const char* className, const T &inCallback)
                {
                    CHECK_NOT_NULL(className);
                    CHECK_NE(std::string(""), className);
                    return SetConstructorInternal(className, CreateFunctionTemplate(m_Isolate, Utils::MakeCallback(inCallback), nullptr, true));
                }

                template <typename T>
                V8ObjectTemplateBuilder &SetValue(const std::string &inName, const T &inCallback)
                {
                    return SetValueMethodInternal(inName, CreateFunctionTemplate(m_Isolate, Utils::MakeCallback(inCallback), m_TypeName));
                }

                template <typename T>
                V8ObjectTemplateBuilder &SetMethod(const std::string &inName, const T &inCallback)
                {
                    return SetValueMethodInternal(inName, CreateFunctionTemplate(m_Isolate, Utils::MakeCallback(inCallback), m_TypeName));
                }

                template <typename G>
                V8ObjectTemplateBuilder &SetReadOnlyProperty(const std::string &inName, const G &inGetter)
                {
                    return SetPropertyInternal(inName, CreateFunctionTemplate(m_Isolate, Utils::MakeCallback(inGetter), m_TypeName),
                                               v8::Local<v8::FunctionTemplate>());
                }

                template <typename G, typename S>
                V8ObjectTemplateBuilder &SetProperty(const std::string &inName, const G &inGetter, const S &inSetter)
                {
                    return SetPropertyInternal(inName, CreateFunctionTemplate(m_Isolate, Utils::MakeCallback(inGetter), m_TypeName),
                                               CreateFunctionTemplate(m_Isolate, Utils::MakeCallback(inSetter), m_TypeName));
                }

                v8::Local<v8::ObjectTemplate> Build();

            private:
                V8ObjectTemplateBuilder &SetConstructorInternal(const std::string &inName, v8::Local<v8::FunctionTemplate> inConstructor);
                V8ObjectTemplateBuilder &SetValueMethodInternal(const std::string &inName, v8::Local<v8::Data> inValue);
                V8ObjectTemplateBuilder &SetPropertyInternal(const std::string &inName, v8::Local<v8::FunctionTemplate> inGetter,
                                                             v8::Local<v8::FunctionTemplate> inSetter);

                v8::Isolate* m_Isolate;
                v8::Local<v8::ObjectTemplate> m_Template;
                bool m_ConstructorAllowed;

                const char *m_TypeName = nullptr;
            };
        }
    }
}

#endif