/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/annotations/deprecation.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init google_protobuf_descriptor_proto_upbdefinit;

static const char descriptor[283] =
  "\n#envoy/annotations/deprecation.proto\022\021e"
  "nvoy.annotations\032 google/protobuf/descri"
  "ptor.proto:T\n\025disallowed_by_default\022\035.go"
  "ogle.protobuf.FieldOptions\030\347\255\256Z \001(\010R\023dis"
  "allowedByDefault:a\n\032disallowed_by_defaul"
  "t_enum\022!.google.protobuf.EnumValueOption"
  "s\030\365\316\266! \001(\010R\027disallowedByDefaultEnumb\006pro"
  "to3";

static upb_def_init *deps[2] = {
  &google_protobuf_descriptor_proto_upbdefinit,
  NULL
};

upb_def_init envoy_annotations_deprecation_proto_upbdefinit = {
  deps,
  NULL,
  "envoy/annotations/deprecation.proto",
  UPB_STRVIEW_INIT(descriptor, 283)
};
