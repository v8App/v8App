// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_OBJECT_TEMPLATE_BUILDER_H__
#define __V8_OBJECT_TEMPLATE_BUILDER_H__

#include <functional>

#include "v8/v8.h"

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
            /**
             * Class to maek it easier to hook up a cpp class to a v8 object.
             * 
             * Note: you should set the constructor first before setting any other 
             * function as the contructor will clear the already registered functions.
            */
            class V8ObjectTemplateBuilder
            {
            public:
                explicit V8ObjectTemplateBuilder(V8Isolate* inIsolate, V8LObject inGlobal, const char* inTypeName = nullptr);

                V8ObjectTemplateBuilder(const V8ObjectTemplateBuilder &inTemplate);
                ~V8ObjectTemplateBuilder();

                // Use the classes typename for the v8 symbol name
                template <typename T>
                V8ObjectTemplateBuilder &SetConstuctor(const T &inCallback, V8LContext inContext)
                {
                    return SetConstuctor(m_TypeName, inCallback, inContext);
                }

                //allows specifying the symbol name for setting the v8 symbol for the function 
                template <typename T>
                V8ObjectTemplateBuilder &SetConstuctor(const char* className, const T &inCallback, V8LContext inContext)
                {
                    CHECK_NOT_NULL(className);
                    CHECK_NE(std::string(""), className);
                    return SetConstructorInternal(className, CreateFunctionTemplate(m_Isolate, Utils::MakeCallback(inCallback), nullptr, true), inContext);
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
                                               V8LFuncTpl());
                }

                template <typename G, typename S>
                V8ObjectTemplateBuilder &SetProperty(const std::string &inName, const G &inGetter, const S &inSetter)
                {
                    return SetPropertyInternal(inName, CreateFunctionTemplate(m_Isolate, Utils::MakeCallback(inGetter), m_TypeName),
                                               CreateFunctionTemplate(m_Isolate, Utils::MakeCallback(inSetter), m_TypeName));
                }

                V8LObjTpl Build();

            private:
                V8ObjectTemplateBuilder &SetConstructorInternal(const std::string &inName, V8LFuncTpl inConstructor, V8LContext inContext);
                V8ObjectTemplateBuilder &SetValueMethodInternal(const std::string &inName, V8LData inValue);
                V8ObjectTemplateBuilder &SetPropertyInternal(const std::string &inName, V8LFuncTpl inGetter,
                                                             V8LFuncTpl inSetter);

                V8Isolate* m_Isolate;
                V8LObject m_Global;
                V8LObjTpl m_Template;
                bool m_ConstructorAllowed;
                const char* m_TypeName;
            };
        }
    }
}

#endif