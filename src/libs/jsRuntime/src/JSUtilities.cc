// Copyright 2020 The v8App Authors. All rights reserved.
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
            v8::Local<v8::String> GetSourceLine(v8::Isolate *inIsolate, v8::Local<v8::Message> &inMessage)
            {
                v8::MaybeLocal<v8::String> maybe = inMessage->GetSourceLine(inIsolate->GetCurrentContext());
                v8::Local<v8::String> line;
                return maybe.ToLocal(&line) ? line : v8::String::Empty(inIsolate);
            }

            std::string GetStackTrace(V8LocalContext inContext, v8::TryCatch &inTryCatch)
            {
                V8Isolate* isolate  = inContext->GetIsolate();
                if (inTryCatch.HasCaught() == false)
                {
                    return "";
                }

                V8LocalValue exception = inTryCatch.Exception();
                v8::String::Utf8Value exception_str(isolate, exception);

                const char *cStr = *exception_str ? *exception_str : "<string conversion failed>";
                std::stringstream messageBuilder;
                v8::Local<v8::Message> message = inTryCatch.Message();
                if (message.IsEmpty())
                {
                    messageBuilder << cStr << std::endl;
                }
                else
                {
                    messageBuilder << V8ToString(isolate, message->Get()) << std::endl
                                   << V8ToString(isolate, message->GetScriptOrigin().ResourceName()) << ":" 
                                   << message->GetLineNumber(inContext).FromMaybe(-1) << ":" << cStr
                                   << std::endl;

                    v8::Local<v8::StackTrace> trace = message->GetStackTrace();
                    if (trace.IsEmpty() == false)
                    {
                        int length = trace->GetFrameCount();
                        for (int idx = 0; idx < length; idx++)
                        {
                            v8::Local<v8::StackFrame> frame = trace->GetFrame(isolate, idx);
                            messageBuilder << V8ToString(isolate, frame->GetScriptName()) << ":"
                                           << frame->GetLineNumber() << ":" << frame->GetColumn() << ":"
                                           << V8ToString(isolate, frame->GetFunctionName()) << std::endl;
                        }
                    }
                }
                return messageBuilder.str();
            }

            void ThrowV8Error(v8::Isolate *inIsolate, V8Errors inType, std::string inMessage)
            {
                v8::Local<v8::Value> error;
                v8::Local<v8::String> v8Message = JSUtilities::StringToV8(inIsolate, inMessage);

                switch (inType)
                {
                case V8Errors::RangeError:
                    error = v8::Exception::RangeError(v8Message);
                    break;
                case V8Errors::ReferenceError:
                    error = v8::Exception::ReferenceError(v8Message);
                    break;
                case V8Errors::SyntaxError:
                    error = v8::Exception::SyntaxError(v8Message);
                    break;
                case V8Errors::TypeError:
                    error = v8::Exception::TypeError(v8Message);
                    break;
                case V8Errors::WasmCompileError:
                    error = v8::Exception::WasmCompileError(v8Message);
                    break;
                case V8Errors::WasmLinkError:
                    error = v8::Exception::WasmLinkError(v8Message);
                    break;
                case V8Errors::WasmRuntimeError:
                    error = v8::Exception::WasmRuntimeError(v8Message);
                    break;

                default:
                    error = v8::Exception::Error(v8Message);
                }

                inIsolate->ThrowException(error);
            }

            v8::Local<v8::String> CreateSymbol(v8::Isolate *inIsolate, const std::string &inString)
            {
                return v8::String::NewFromUtf8(inIsolate, inString.c_str(), v8::NewStringType::kInternalized, inString.length()).ToLocalChecked();
            }

            v8::Local<v8::String> CreateSymbol(v8::Isolate *inIsolate, const std::u16string &inString)
            {
                return v8::String::NewFromTwoByte(inIsolate, reinterpret_cast<const uint16_t *>(inString.c_str()), v8::NewStringType::kInternalized, inString.length()).ToLocalChecked();
            }

            std::string V8ToString(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue)
            {
                if (inValue.IsEmpty() || inValue->IsString() == false)
                {
                    return std::string();
                }
                std::string outValue;
                v8::Local<v8::String> v8Str = v8::Local<v8::String>::Cast(inValue);
                int length = v8Str->Utf8Length(inIsolate);
                outValue.resize(length);
                v8Str->WriteUtf8(inIsolate, &(outValue)[0], length, nullptr, v8::String::NO_NULL_TERMINATION);
                return outValue;
            }

            std::u16string V8ToU16String(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue)
            {
                if (inValue.IsEmpty() || inValue->IsString() == false)
                {
                    return std::u16string();
                }
                std::u16string outValue;

                v8::Local<v8::String> v8Str = v8::Local<v8::String>::Cast(inValue);
                outValue.resize(v8Str->Length());
                v8Str->Write(inIsolate, reinterpret_cast<uint16_t *>(&(outValue)[0]), 0, v8Str->Length(), v8::String::NO_NULL_TERMINATION);
                return outValue;
            }

            v8::Local<v8::String> StringToV8(v8::Isolate *inIsolate, const std::string &inString, v8::NewStringType inType)
            {
                return v8::String::NewFromUtf8(inIsolate, inString.c_str(), inType, inString.length()).ToLocalChecked();
            }

            v8::Local<v8::String> U16StringToV8(v8::Isolate *inIsolate, const std::u16string &inString, v8::NewStringType inType)
            {
                return v8::String::NewFromTwoByte(inIsolate, reinterpret_cast<const uint16_t *>(inString.c_str()), inType, inString.length()).ToLocalChecked();
            }

            v8::Local<v8::String> ReadScriptFile(v8::Isolate *inIsolate, std::filesystem::path inFilename)
            {
                if (std::filesystem::exists(inFilename) == false)
                {
                    return v8::Local<v8::String>();
                }

                std::ifstream fileStream(inFilename, std::ios::ate | std::ios::in | std::ios::binary);
                if (fileStream.is_open() == false)
                {
                    return v8::Local<v8::String>();
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
