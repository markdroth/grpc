/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/api/v2/core/backoff.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init google_protobuf_duration_proto_upbdefinit;
extern upb_def_init udpa_annotations_migrate_proto_upbdefinit;
extern upb_def_init udpa_annotations_status_proto_upbdefinit;
extern upb_def_init validate_validate_proto_upbdefinit;
extern const upb_msglayout envoy_api_v2_core_BackoffStrategy_msginit;

static const upb_msglayout *layouts[1] = {
  &envoy_api_v2_core_BackoffStrategy_msginit,
};

static const char descriptor[439] =
  "\n\037envoy/api/v2/core/backoff.proto\022\021envoy"
  ".api.v2.core\032\036google/protobuf/duration.p"
  "roto\032\036udpa/annotations/migrate.proto\032\035ud"
  "pa/annotations/status.proto\032\027validate/va"
  "lidate.proto\"\251\001\n\017BackoffStrategy\022N\n\rbase"
  "_interval\030\001 \001(\0132\031.google.protobuf.Durati"
  "onB\016\372B\013\252\001\010\010\0012\004\020\300\204=R\014baseInterval\022F\n\014max_"
  "interval\030\002 \001(\0132\031.google.protobuf.Duratio"
  "nB\010\372B\005\252\001\002*\000R\013maxIntervalBU\n\037io.envoyprox"
  "y.envoy.api.v2.coreB\014BackoffProtoP\001\362\230\376\217\005"
  "\026\022\024envoy.config.core.v3\272\200\310\321\006\002\020\001b\006proto3";

static upb_def_init *deps[5] = {
  &google_protobuf_duration_proto_upbdefinit,
  &udpa_annotations_migrate_proto_upbdefinit,
  &udpa_annotations_status_proto_upbdefinit,
  &validate_validate_proto_upbdefinit,
  NULL
};

upb_def_init envoy_api_v2_core_backoff_proto_upbdefinit = {
  deps,
  layouts,
  "envoy/api/v2/core/backoff.proto",
  UPB_STRVIEW_INIT(descriptor, 439)
};
