/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/api/v2/route.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init envoy_api_v2_core_base_proto_upbdefinit;
extern upb_def_init envoy_api_v2_core_config_source_proto_upbdefinit;
extern upb_def_init envoy_api_v2_route_route_components_proto_upbdefinit;
extern upb_def_init google_protobuf_wrappers_proto_upbdefinit;
extern upb_def_init udpa_annotations_migrate_proto_upbdefinit;
extern upb_def_init udpa_annotations_status_proto_upbdefinit;
extern upb_def_init validate_validate_proto_upbdefinit;
extern const upb_msglayout envoy_api_v2_RouteConfiguration_msginit;
extern const upb_msglayout envoy_api_v2_Vhds_msginit;

static const upb_msglayout *layouts[2] = {
  &envoy_api_v2_RouteConfiguration_msginit,
  &envoy_api_v2_Vhds_msginit,
};

static const char descriptor[1185] =
  "\n\030envoy/api/v2/route.proto\022\014envoy.api.v2"
  "\032\034envoy/api/v2/core/base.proto\032%envoy/ap"
  "i/v2/core/config_source.proto\032)envoy/api"
  "/v2/route/route_components.proto\032\036google"
  "/protobuf/wrappers.proto\032\036udpa/annotatio"
  "ns/migrate.proto\032\035udpa/annotations/statu"
  "s.proto\032\027validate/validate.proto\"\335\005\n\022Rou"
  "teConfiguration\022\022\n\004name\030\001 \001(\tR\004name\022D\n\rv"
  "irtual_hosts\030\002 \003(\0132\037.envoy.api.v2.route."
  "VirtualHostR\014virtualHosts\022&\n\004vhds\030\t \001(\0132"
  "\022.envoy.api.v2.VhdsR\004vhds\022D\n\025internal_on"
  "ly_headers\030\003 \003(\tB\020\372B\r\222\001\n\"\010r\006\300\001\001\310\001\000R\023inte"
  "rnalOnlyHeaders\022f\n\027response_headers_to_a"
  "dd\030\004 \003(\0132$.envoy.api.v2.core.HeaderValue"
  "OptionB\t\372B\006\222\001\003\020\350\007R\024responseHeadersToAdd\022"
  "M\n\032response_headers_to_remove\030\005 \003(\tB\020\372B\r"
  "\222\001\n\"\010r\006\300\001\001\310\001\000R\027responseHeadersToRemove\022d"
  "\n\026request_headers_to_add\030\006 \003(\0132$.envoy.a"
  "pi.v2.core.HeaderValueOptionB\t\372B\006\222\001\003\020\350\007R"
  "\023requestHeadersToAdd\022K\n\031request_headers_"
  "to_remove\030\010 \003(\tB\020\372B\r\222\001\n\"\010r\006\300\001\001\310\001\000R\026reque"
  "stHeadersToRemove\022L\n#most_specific_heade"
  "r_mutations_wins\030\n \001(\010R\037mostSpecificHead"
  "erMutationsWins\022G\n\021validate_clusters\030\007 \001"
  "(\0132\032.google.protobuf.BoolValueR\020validate"
  "Clusters\"V\n\004Vhds\022N\n\rconfig_source\030\001 \001(\0132"
  "\037.envoy.api.v2.core.ConfigSourceB\010\372B\005\212\001\002"
  "\020\001R\014configSourceBO\n\032io.envoyproxy.envoy."
  "api.v2B\nRouteProtoP\001\362\230\376\217\005\027\022\025envoy.config"
  ".route.v3\272\200\310\321\006\002\020\001b\006proto3";

static upb_def_init *deps[8] = {
  &envoy_api_v2_core_base_proto_upbdefinit,
  &envoy_api_v2_core_config_source_proto_upbdefinit,
  &envoy_api_v2_route_route_components_proto_upbdefinit,
  &google_protobuf_wrappers_proto_upbdefinit,
  &udpa_annotations_migrate_proto_upbdefinit,
  &udpa_annotations_status_proto_upbdefinit,
  &validate_validate_proto_upbdefinit,
  NULL
};

upb_def_init envoy_api_v2_route_proto_upbdefinit = {
  deps,
  layouts,
  "envoy/api/v2/route.proto",
  UPB_STRVIEW_INIT(descriptor, 1185)
};
