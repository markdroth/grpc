/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/api/v2/core/event_service_config.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init envoy_api_v2_core_grpc_service_proto_upbdefinit;
extern upb_def_init udpa_annotations_migrate_proto_upbdefinit;
extern upb_def_init udpa_annotations_status_proto_upbdefinit;
extern upb_def_init validate_validate_proto_upbdefinit;
extern const upb_msglayout envoy_api_v2_core_EventServiceConfig_msginit;

static const upb_msglayout *layouts[1] = {
  &envoy_api_v2_core_EventServiceConfig_msginit,
};

static const char descriptor[420] =
  "\n,envoy/api/v2/core/event_service_config"
  ".proto\022\021envoy.api.v2.core\032$envoy/api/v2/"
  "core/grpc_service.proto\032\036udpa/annotation"
  "s/migrate.proto\032\035udpa/annotations/status"
  ".proto\032\027validate/validate.proto\"y\n\022Event"
  "ServiceConfig\022C\n\014grpc_service\030\001 \001(\0132\036.en"
  "voy.api.v2.core.GrpcServiceH\000R\013grpcServi"
  "ceB\036\n\027config_source_specifier\022\003\370B\001B`\n\037io"
  ".envoyproxy.envoy.api.v2.coreB\027EventServ"
  "iceConfigProtoP\001\362\230\376\217\005\026\022\024envoy.config.cor"
  "e.v3\272\200\310\321\006\002\020\001b\006proto3";

static upb_def_init *deps[5] = {
  &envoy_api_v2_core_grpc_service_proto_upbdefinit,
  &udpa_annotations_migrate_proto_upbdefinit,
  &udpa_annotations_status_proto_upbdefinit,
  &validate_validate_proto_upbdefinit,
  NULL
};

upb_def_init envoy_api_v2_core_event_service_config_proto_upbdefinit = {
  deps,
  layouts,
  "envoy/api/v2/core/event_service_config.proto",
  UPB_STRVIEW_INIT(descriptor, 420)
};
