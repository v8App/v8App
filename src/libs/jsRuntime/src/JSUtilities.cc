// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <sstream>
#include <fstream>

#include "V8Types.h"
#include "JSUtilities.h"
#include "CppBridge/V8TypeConverter.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace JSUtilities
        {
            V8LString GetSourceLine(V8Isolate *inIsolate, V8LMessage &inMessage)
            {
                V8MBLString maybe = inMessage->GetSourceLine(inIsolate->GetCurrentContext());
                V8LString line;
                return maybe.ToLocal(&line) ? line : V8String::Empty(inIsolate);
            }

            std::string GetStackTrace(V8Isolate *inIsolate, V8TryCatch &inTryCatch, std::string inResourceName)
            {
                if (inTryCatch.HasCaught() == false)
                {
                    return "";
                }
                V8LValue exception = inTryCatch.Exception();
                V8String::Utf8Value exception_str(inIsolate, exception);

                const char *cStr = *exception_str ? *exception_str : "<string conversion failed>";
                std::stringstream messageBuilder;
                V8LMessage message = inTryCatch.Message();
                if (message.IsEmpty())
                {
                    messageBuilder << cStr << std::endl;
                }
                else
                {
                    V8LContext context = inIsolate->GetCurrentContext();

                    std::string resourceName = V8ToString(inIsolate, message->GetScriptOrigin().ResourceName());
                    if (resourceName == "")
                    {
                        resourceName = inResourceName;
                    }
                    int sourceLine = -1;
                    if (context.IsEmpty() == false)
                    {
                        message->GetLineNumber(context).To(&sourceLine);
                    }
                    messageBuilder << V8ToString(inIsolate, message->Get()) << std::endl
                                   << resourceName << ":" << sourceLine
                                   << ":" << cStr
                                   << std::endl;

                    V8LStackTrace trace = message->GetStackTrace();
                    if (trace.IsEmpty() == false)
                    {
                        int length = trace->GetFrameCount();
                        for (int idx = 0; idx < length; idx++)
                        {
                            V8LStackFrame frame = trace->GetFrame(inIsolate, idx);
                            messageBuilder << V8ToString(inIsolate, frame->GetScriptName()) << ":"
                                           << frame->GetLineNumber() << ":" << frame->GetColumn() << ":"
                                           << V8ToString(inIsolate, frame->GetFunctionName()) << std::endl;
                        }
                    }
                }
                return messageBuilder.str();
            }

            void ThrowV8Error(V8Isolate *inIsolate, V8Errors inType, std::string inMessage)
            {
                V8LValue error;
                V8LString v8Message = JSUtilities::StringToV8(inIsolate, inMessage);

                switch (inType)
                {
                case V8Errors::RangeError:
                    error = V8Exception::RangeError(v8Message);
                    break;
                case V8Errors::ReferenceError:
                    error = V8Exception::ReferenceError(v8Message);
                    break;
                case V8Errors::SyntaxError:
                    error = V8Exception::SyntaxError(v8Message);
                    break;
                case V8Errors::TypeError:
                    error = V8Exception::TypeError(v8Message);
                    break;
                case V8Errors::WasmCompileError:
                    error = V8Exception::WasmCompileError(v8Message);
                    break;
                case V8Errors::WasmLinkError:
                    error = V8Exception::WasmLinkError(v8Message);
                    break;
                case V8Errors::WasmRuntimeError:
                    error = V8Exception::WasmRuntimeError(v8Message);
                    break;

                default:
                    error = V8Exception::Error(v8Message);
                }

                inIsolate->ThrowException(error);
            }

            V8LString CreateSymbol(V8Isolate *inIsolate, const std::string &inString)
            {
                return V8String::NewFromUtf8(inIsolate, inString.c_str(), v8::NewStringType::kInternalized, inString.length()).ToLocalChecked();
            }

            V8LString CreateSymbol(V8Isolate *inIsolate, const std::u16string &inString)
            {
                return V8String::NewFromTwoByte(inIsolate, reinterpret_cast<const uint16_t *>(inString.c_str()), v8::NewStringType::kInternalized, inString.length()).ToLocalChecked();
            }

            std::string V8ToString(V8Isolate *inIsolate, V8LValue inValue)
            {
                if (inValue.IsEmpty() || inValue->IsString() == false)
                {
                    return std::string();
                }
                std::string outValue;
                V8LString v8Str = V8LString::Cast(inValue);
                int length = v8Str->Utf8Length(inIsolate);
                outValue.resize(length);
                v8Str->WriteUtf8(inIsolate, &(outValue)[0], length, nullptr, V8String::NO_NULL_TERMINATION);
                return outValue;
            }

            std::u16string V8ToU16String(V8Isolate *inIsolate, V8LValue inValue)
            {
                if (inValue.IsEmpty() || inValue->IsString() == false)
                {
                    return std::u16string();
                }
                std::u16string outValue;

                V8LString v8Str = V8LString::Cast(inValue);
                outValue.resize(v8Str->Length());
                v8Str->Write(inIsolate, reinterpret_cast<uint16_t *>(&(outValue)[0]), 0, v8Str->Length(), V8String::NO_NULL_TERMINATION);
                return outValue;
            }

            V8LString StringToV8(V8Isolate *inIsolate, const std::string &inString, v8::NewStringType inType)
            {
                return V8String::NewFromUtf8(inIsolate, inString.c_str(), inType, inString.length()).ToLocalChecked();
            }

            V8LString U16StringToV8(V8Isolate *inIsolate, const std::u16string &inString, v8::NewStringType inType)
            {
                return V8String::NewFromTwoByte(inIsolate, reinterpret_cast<const uint16_t *>(inString.c_str()), inType, inString.length()).ToLocalChecked();
            }

            V8LString ReadScriptFile(V8Isolate *inIsolate, std::filesystem::path inFilename)
            {
                if (std::filesystem::exists(inFilename) == false)
                {
                    return V8LString();
                }

                std::ifstream fileStream(inFilename, std::ios::ate | std::ios::in | std::ios::binary);
                if (fileStream.is_open() == false)
                {
                    return V8LString();
                }

                std::streampos fileSize = fileStream.tellg();
                fileStream.seekg(0, std::ios::beg);
                char *contents = new char[fileSize];
                fileStream.read(contents, fileSize);
                fileStream.close();

                // TODO:: attempt to detect encoding of the file for unicode
                // for now just assume ascii/utf8

                return StringToV8(inIsolate, contents);
            }

        }
    }
}
