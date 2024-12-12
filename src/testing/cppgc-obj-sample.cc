// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "libplatform/libplatform.h"
#include "v8-context.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-primitive.h"
#include "v8-script.h"
#include "cppgc/garbage-collected.h"
#include "v8-cppgc.h"
#include "v8/cppgc/allocation.h"
#include "v8-template.h"
#include "v8-exception.h"
#include "v8-function.h"

v8::Global<v8::FunctionTemplate> cppObjFuncTpl;
std::unique_ptr<v8::SnapshotCreator> creator;
uint16_t heapId{1};

/**
 * Internal fields are
 * 0 = cppHeapId
 * 1 = type info,
 * 2 = pointer to cpp object
 */
class CppObject : public cppgc::GarbageCollected<CppObject>
{
public:
  CppObject() {}
  virtual ~CppObject()
  {
    std::cout << "Destructor for CppObject Called" << std::endl;
    value = 0;
  }

  virtual void Trace(cppgc::Visitor *visitor) const {}

  static void Constructor(const v8::FunctionCallbackInfo<v8::Value> &info)
  {
    v8::Isolate *isolate = info.GetIsolate();
    v8::CppHeap *heap = isolate->GetCppHeap();

    CppObject *obj = cppgc::MakeGarbageCollected<CppObject>(heap->GetAllocationHandle());
    v8::Isolate::Scope iScope(isolate);
    v8::HandleScope hScope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::FunctionTemplate> tpl = cppObjFuncTpl.Get(isolate);
    v8::Local<v8::Object> jsObj;
    if (tpl->PrototypeTemplate()->NewInstance(context).ToLocal(&jsObj) == false)
    {
      std::cout << "Failed to create the prototype instance for CppObject" << std::endl;
      isolate->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8Literal(isolate, "Filed to create object")));
    }
    int indexes[] = {0, 1, 2};
    void *values[] = {&heapId, &CppObject::typeName, obj};
    jsObj->SetAlignedPointerInInternalFields(3, indexes, values);
    info.GetReturnValue().Set(jsObj);
  }

  static void SetValue(const v8::FunctionCallbackInfo<v8::Value> &info)
  {
    v8::Isolate *isolate = info.GetIsolate();
    v8::HandleScope hScope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    if (info.Length() != 1)
    {
      return;
    }
    v8::Local<v8::Value> v8Int = info[0];
    v8::Local<v8::Number> maybeNum;

    if (v8Int->IsNumber() == false)
    {
      return;
    }
    int32_t intValue = v8Int->ToInt32(context).ToLocalChecked()->Int32Value(context).FromJust();
    v8::Local<v8::Object> jsObj = info.Holder();
    if (&CppObject::typeName != jsObj->GetAlignedPointerFromInternalField(1))
    {
      return;
    }
    CppObject *obj = reinterpret_cast<CppObject *>(jsObj->GetAlignedPointerFromInternalField(1));
    obj->value = intValue;
    std::cout << "CppObject Value set to: " << obj->value << std::endl;
  }

  static void GetValue(const v8::FunctionCallbackInfo<v8::Value> &info)
  {
    v8::Isolate *isolate = info.GetIsolate();
    v8::HandleScope hScope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Object> jsObj = info.Holder();
    if (CppObject::typeName != jsObj->GetAlignedPointerFromInternalField(1))
    {
      return;
    }
    CppObject *obj = reinterpret_cast<CppObject *>(jsObj->GetAlignedPointerFromInternalField(2));
    v8::Local<v8::Integer> v8Num = v8::Integer::New(isolate, obj->value);
    info.GetReturnValue().Set(v8Num);
  }

  static inline const char *typeName{"CpObject"};
  int value{5};
};

intptr_t external_refs[]{(intptr_t)&CppObject::Constructor, (intptr_t)&CppObject::SetValue, (intptr_t)&CppObject::GetValue};

v8::StartupData SerializeInternalField(v8::Local<v8::Object> inHolder, int inIndex, void *inData)
{
  const char *data;
  if (inIndex == 1)
  {
    char *typeName = new char[]{"CppObject"};
    return {typeName, (int)std::strlen(CppObject::typeName)};
  }
  if (inIndex == 2)
  {
    std::cout << "CppObject Serlializer called" << std::endl;
    size_t intSize = sizeof(int);
    CppObject *obj = reinterpret_cast<CppObject *>(inHolder->GetAlignedPointerFromInternalField(inIndex));

    if (obj->value != 100)
    {
      std::cout << "CpObject value wasn't 100 when serializing" << std::endl;
    }
    char *value = new char[intSize]{};
    std::memcpy((void *)value, &obj->value, intSize);
    return {value, (int)intSize};
  }
  return {nullptr, 0};
}

void DeserializeInternalField(v8::Local<v8::Object> inHolder, int inIndex, v8::StartupData inPayload, void *inData)
{
  if (inIndex == 1)
  {
    int indexes[] = {0, 1};
    void *values[] = {&heapId, &CppObject::typeName};
    inHolder->SetAlignedPointerInInternalFields(2, indexes, values);
    return;
  }
  if (inIndex == 1)
  {
    v8::Isolate *isolate = inHolder->GetIsolate();
    size_t intSize = sizeof(int);
    if (inPayload.raw_size != intSize)
    {
      std::cout << "Raw data size for CppObject value was not size of int";
      return;
    }
    v8::CppHeap *heap = isolate->GetCppHeap();
    CppObject *obj = cppgc::MakeGarbageCollected<CppObject>(heap->GetAllocationHandle());
    obj->value = *((int *)inPayload.data);
    inHolder->SetAlignedPointerInInternalField(2, obj);
  }
}

void CreateFunctionTemplate(v8::Isolate *isolate, v8::Local<v8::Context> context)
{
  v8::Local<v8::FunctionTemplate> funcTpl = v8::FunctionTemplate::New(isolate);
  v8::Local<v8::FunctionTemplate> setter = v8::FunctionTemplate::New(isolate, &CppObject::SetValue);
  v8::Local<v8::FunctionTemplate> getter = v8::FunctionTemplate::New(isolate, &CppObject::GetValue);

  funcTpl->SetCallHandler(&CppObject::Constructor);
  funcTpl->SetLength(1);
  funcTpl->SetClassName(v8::String::NewFromUtf8Literal(isolate, "CppObject"));
  cppObjFuncTpl.Reset(isolate, funcTpl);

  funcTpl->PrototypeTemplate()->SetInternalFieldCount(3);
  funcTpl->PrototypeTemplate()->SetAccessorProperty(
      v8::String::NewFromUtf8Literal(isolate, "value"),
      getter,
      setter);

  v8::Local<v8::Object> global = context->Global();
  global->Set(context, v8::String::NewFromUtf8Literal(isolate, "CppObject"), funcTpl->GetFunction(context).ToLocalChecked());
}

v8::Isolate *CreateIsolate(v8::Platform *platform, v8::StartupData *snapData, bool snapshot)
{
  // Create a new Isolate and make it the current one.
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  create_params.external_references = external_refs;
  create_params.snapshot_blob = snapData;

  std::unique_ptr<v8::CppHeap> cppHeap = v8::CppHeap::Create(
      platform,
      v8::CppHeapCreateParams({}, v8::WrapperDescriptor(1, 2, 0)));
  create_params.cpp_heap = cppHeap.get();
  cppHeap.release();

  v8::Isolate *isolate = v8::Isolate::Allocate();
  if (snapshot)
  {
    creator = std::make_unique<v8::SnapshotCreator>(isolate, create_params);
  }
  else
  {
    v8::Isolate::Initialize(isolate, create_params);
  }
  return isolate;
}

int main(int argc, char *argv[])
{
  // Initialize V8.
  const char *icuLoc = std::getenv("V8_ICU_DATA");
  const char *snapData = std::getenv("V8_SNAPSHOT_BIN");
  if (icuLoc == nullptr)
  {
    std::cout << "Failed to find the V8_ICU_DATA env var";
    std::exit(1);
  }
  if (snapData == nullptr)
  {
    std::cout << "Failed to find the V8_ICU_DATA env var";
    std::exit(1);
  }

  std::cout << snapData << std::endl;

  v8::V8::InitializeICUDefaultLocation(icuLoc);
  v8::V8::InitializeExternalStartupData(snapData);

  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  cppgc::InitializeProcess(platform->GetPageAllocator());

  v8::Isolate *isolate = CreateIsolate(platform.get(), nullptr, true);

  v8::StartupData customBlob;

  {
    v8::Isolate::Scope isolate_scope(isolate);
    {
      // Create a stack-allocated handle scope.
      v8::HandleScope handle_scope(isolate);

      // Create a new context.
      v8::Local<v8::Context>
          context = v8::Context::New(isolate);

      CreateFunctionTemplate(isolate, context);

      // Enter the context for compiling and running the hello world script.
      v8::Context::Scope cScope(context);
      {
        const char csource1[] = R"(
                    var snapObj = new CppObject();
                    snapObj.value = 100;
                )";

        // Create a string containing the JavaScript source code.
        v8::Local<v8::String> source =
            v8::String::NewFromUtf8Literal(isolate, csource1);

        // Compile the source code.
        v8::Local<v8::Script> script =
            v8::Script::Compile(context, source).ToLocalChecked();

        script->Run(context).ToLocalChecked();
      }
      creator->SetDefaultContext(context, v8::SerializeInternalFieldsCallback(&SerializeInternalField, nullptr));
    }
  }

  cppObjFuncTpl.Reset();
  std::cout << "Creating Custom snapshot" << std::endl;
  customBlob = creator->CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kKeep);
  creator.reset();
  isolate->Dispose();
  std::cout << "Finished Custom snapshot" << std::endl;
  std::cout << "Snapshot Isolate disposed of" << std::endl;

  if (customBlob.raw_size == 0)
  {
    std::cout << "Faile to create the snapshot" << std::endl;
    return 1;
  }

  std::cout << "Creating new isolate using custom snapshot" << std::endl;

  isolate = CreateIsolate(platform.get(), &customBlob, false);
  {
    v8::Isolate::Scope iScope(isolate);
    v8::HandleScope hScope(isolate);

    v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr, {}, {}, v8::DeserializeInternalFieldsCallback(&DeserializeInternalField, nullptr));
    // CreateFunctionTemplate(isolate, context);
    {
      v8::Context::Scope cScope(context);
      v8::TryCatch tryCatch(isolate);
      const char csource1[] = R"(
                    snapObj.value = 200;
                    snapObj = undefined;
                )";

      // Create a string containing the JavaScript source code.
      v8::Local<v8::String> source =
          v8::String::NewFromUtf8Literal(isolate, csource1);

      // Compile the source code.
      v8::Local<v8::Script> script =
          v8::Script::Compile(context, source).ToLocalChecked();

      script->Run(context);

      if (tryCatch.HasCaught())
      {
        std::cout << "Got script error on custom snapshot" << std::endl;
      }
    }
  }


  std::cout << "Disposing of custom snapshot isolate" << std::endl;
  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  return 0;
}
