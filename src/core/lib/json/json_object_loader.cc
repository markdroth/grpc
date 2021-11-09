// Copyright 2020 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <grpc/support/port_platform.h>

#include "src/core/lib/json/json_object_loader.h"

#include "absl/strings/str_cat.h"

namespace grpc_core {

void ErrorList::AddError(absl::string_view error) {
  std::string message = "field:";
  for (const auto& field : fields_) {
    message += field;
  }
  errors_.emplace_back(absl::StrCat(message, " error:", error));
}

void ErrorList::PushField(absl::string_view ext) {
  fields_.emplace_back(std::string(ext));
}

void ErrorList::PopField() { fields_.pop_back(); }

namespace json_detail {

template <typename T>
GPR_ATTRIBUTE_NOINLINE void ConvertNumber(const std::string& value, void* dest,
                                          ErrorList* error_list) {
  if (!absl::SimpleAtoi(value, static_cast<T*>(dest))) {
    error_list->AddError("failed to parse.");
  }
}

template <typename T>
GPR_ATTRIBUTE_NOINLINE void ConvertString(const std::string& value, void* dest,
                                          ErrorList*) {
  *static_cast<T*>(dest) = value;
}

void TypeRef::Load(const Json::Object& json, void* dest,
                   ErrorList* errors) const {
  char* dest_chr = static_cast<char*>(dest);
  for (size_t i = 0; i < count; i++) {
    ScopedField field(errors, absl::StrCat(".", elements[i].name));
    auto it = json.find(elements[i].name);
    if (it == json.end()) {
      if (!elements[i].optional) errors->AddError("does not exist.");
      continue;
    }
    void* member_ptr = dest_chr + elements[i].member_offset;
    const Json& value = it->second;
    switch (elements[i].type) {
      case Element::kInt32:
      case Element::kUint32:
      case Element::kString:
        LoadScalar(value, elements[i].type, member_ptr, errors);
        break;
      case Element::kVector:
        LoadVector(value, elements[i].type_data, member_ptr, errors);
        break;
      case Element::kMap:
        LoadMap(value, elements[i].type_data, member_ptr, errors);
        break;
    }
  }
}

void TypeRef::LoadScalar(const Json& json, Element::Type type, void* dest,
                         ErrorList* errors) const {
  WithLoaderForTypeData(type, errors,
                        [&](const TypeVtable*, LoadFn load)
                            GPR_ATTRIBUTE_NOINLINE { load(json, dest); });
}

void TypeRef::LoadVector(const Json& json, uint8_t type_data, void* dest,
                         ErrorList* errors) const {
  if (json.type() != Json::Type::ARRAY) {
    errors->AddError("is not an array.");
    return;
  }
  const Json::Array& array = json.array_value();
  for (size_t i = 0; i < array.size(); i++) {
    ScopedField array_elem(errors, absl::StrCat("[", i, "]"));
    const Json& elem_json = array[i];
    WithLoaderForTypeData(type_data, errors,
                          [&](const TypeVtable* vtable, LoadFn load)
                              GPR_ATTRIBUTE_NOINLINE {
                                void* p = vtable->create();
                                load(elem_json, p);
                                vtable->push_to_vec(p, dest);
                                vtable->destroy(p);
                              });
  }
}

void TypeRef::LoadMap(const Json& json, uint8_t type_data, void* dest,
                      ErrorList* errors) const {
  if (json.type() != Json::Type::OBJECT) {
    errors->AddError("is not an object.");
    return;
  }
  const Json::Object& object = json.object_value();
  for (const auto& pair : object) {
    ScopedField map_elem(errors, absl::StrCat(".", pair.first));
    const Json& elem_json = pair.second;
    WithLoaderForTypeData(type_data, errors,
                          [&](const TypeVtable* vtable, LoadFn load)
                              GPR_ATTRIBUTE_NOINLINE {
                                void* p = vtable->create();
                                load(elem_json, p);
                                vtable->insert_to_map(pair.first, p, dest);
                                vtable->destroy(p);
                              });
  }
}

void TypeRef::WithLoaderForTypeData(
    uint8_t tag, ErrorList* errors,
    absl::FunctionRef<void(const TypeVtable* vtable, LoadFn load)> fn) const {
  if (tag < Element::kVector) {
    void (*convert_number)(const std::string& value, void* dest,
                           ErrorList* error_list) = nullptr;
    void (*convert_string)(const std::string& value, void* dest,
                           ErrorList* error_list) = nullptr;
    const TypeVtable* vtable = nullptr;
    switch (static_cast<Element::Type>(tag)) {
      case Element::kVector:
      case Element::kMap:
        abort();  // not reachable
      case Element::kInt32:
        vtable = &TypeVtableImpl<int32_t>::vtable;
        convert_number = ConvertNumber<int32_t>;
        break;
      case Element::kUint32:
        vtable = &TypeVtableImpl<uint32_t>::vtable;
        convert_number = ConvertNumber<uint32_t>;
        break;
      case Element::kString:
        vtable = &TypeVtableImpl<std::string>::vtable;
        convert_string = ConvertString<std::string>;
        break;
    }
    if (convert_number != nullptr) {
      fn(vtable, [convert_number, errors](const Json& json, void* dest_ptr) {
        if (json.type() != Json::Type::NUMBER) {
          errors->AddError("is not a number.");
          return;
        }
        convert_number(json.string_value(), dest_ptr, errors);
      });
      return;
    }
    if (convert_string != nullptr) {
      fn(vtable, [convert_string, errors](const Json& json, void* dest_ptr) {
        if (json.type() != Json::Type::STRING) {
          errors->AddError("is not a string.");
          return;
        }
        convert_string(json.string_value(), dest_ptr, errors);
      });
      return;
    }
    abort();  // not reachable
  }
  type_ref_providers[tag - Element::kVector]->WithTypeRef(
      [&](const TypeRef& type_ref) {
        fn(type_ref.vtable, [type_ref, errors](const Json& json, void* dest) {
          if (json.type() != Json::Type::OBJECT) {
            errors->AddError("is not an object.");
            return;
          }
          type_ref.Load(json.object_value(), dest, errors);
        });
      });
}

}  // namespace json_detail
}  // namespace grpc_core
