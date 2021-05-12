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
                explicit V8ObjectTemplateBuilder(IsolateWeakPtr inIsolate, int inNumInternalFields);
                V8ObjectTemplateBuilder(IsolateWeakPtr inIsolate, int inNumInternalFields, const char *inTypeNmae);
                V8ObjectTemplateBuilder(IsolateWeakPtr inIsolate, int inNumInternalFields, const char *inTypeNmae, v8::Local<v8::ObjectTemplate> inTemplate);

                V8ObjectTemplateBuilder(const V8ObjectTemplateBuilder &inTemplate);
                ~V8ObjectTemplateBuilder();

                template <typename T>
                V8ObjectTemplateBuilder &SetValue(const std::string &inName, T inValue)
                {
                    CHECK_EQ(false, m_Isolate.expired());

                    v8::Isolate* isolate = m_Isolate.lock().get();
                    return SetValueMethodInternal(inName, ConvertToV8(isolate, inValue));
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
                V8ObjectTemplateBuilder &SetValueMethodInternal(const std::string &inName, v8::Local<v8::Data> inValue);
                V8ObjectTemplateBuilder &SetPropertyInternal(const std::string &inName, v8::Local<v8::FunctionTemplate> inGetter,
                                                             v8::Local<v8::FunctionTemplate> inSetter);

                IsolateWeakPtr m_Isolate;
                v8::Local<v8::ObjectTemplate> m_Template;

                const char *m_TypeName = nullptr;
            };
        }
    }
}

#endif