// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <sstream>

#include "JSUtilites.h"
#include "CppBridge/V8TypeConverter.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace JSUtilities
        {
            v8::Local<v8::String> GetSourceLine(v8::Isolate *inIsolate, v8::Local<v8::Message> &inMessage)
            {
                v8::MaybeLocal<v8::String> maybe = inMessage->GetSourceLine(inIsolate->GetCurrentContext());
                v8::Local<v8::String> line;
                return maybe.ToLocal(&line) ? line : v8::String::Empty(inIsolate);
            }

            std::string GetStackTrace(v8::Isolate *inIsolate, v8::TryCatch &inTryCatch)
            {
                if (inTryCatch.HasCaught() == false)
                {
                    return "";
                }

                std::stringstream messageBuilder;
                v8::Local<v8::Message> message = inTryCatch.Message();
                messageBuilder << CppBridge::V8ToString(inIsolate, message->Get()) << std::endl
                               << CppBridge::V8ToString(inIsolate, GetSourceLine(inIsolate, message)) << std::endl;

                v8::Local<v8::StackTrace> trace = message->GetStackTrace();
                if (trace.IsEmpty() == false)
                {
                    int length = trace->GetFrameCount();
                    for (int idx = 0; idx < length; idx++)
                    {
                        v8::Local<v8::StackFrame> frame = trace->GetFrame(inIsolate, idx);
                        messageBuilder << CppBridge::V8ToString(inIsolate, frame->GetScriptName()) << ":"
                                       << frame->GetLineNumber() << ":" << frame->GetColumn() << ":"
                                       << CppBridge::V8ToString(inIsolate, frame->GetFunctionName()) << std::endl;
                    }
                }
                return messageBuilder.str();
            }

            void ThrowV8Error(v8::Isolate *inIsolate, V8Errors inType, v8::Local<v8::String> inMessage)
            {
                v8::Local<v8::Value> error;

                switch (inType)
                {
                case V8Errors::RangeError:
                    error = v8::Exception::RangeError(inMessage);
                    break;
                case V8Errors::ReferenceError:
                    error = v8::Exception::ReferenceError(inMessage);
                    break;
                case V8Errors::SyntaxError:
                    error = v8::Exception::SyntaxError(inMessage);
                    break;
                case V8Errors::TypeError:
                    error = v8::Exception::TypeError(inMessage);
                    break;
                case V8Errors::WasmCompileError:
                    error = v8::Exception::WasmCompileError(inMessage);
                    break;
                case V8Errors::WasmLinkError:
                    error = v8::Exception::WasmLinkError(inMessage);
                    break;
                case V8Errors::WasmRuntimeError:
                    error = v8::Exception::WasmRuntimeError(inMessage);
                    break;

                default:
                    error = v8::Exception::Error(inMessage);
                }

                inIsolate->ThrowException(error);
            }
        }
    }
}
