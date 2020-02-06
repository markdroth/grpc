/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/api/v2/discovery.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init envoy_api_v2_core_base_proto_upbdefinit;
extern upb_def_init google_protobuf_any_proto_upbdefinit;
extern upb_def_init google_rpc_status_proto_upbdefinit;
extern const upb_msglayout envoy_api_v2_DiscoveryRequest_msginit;
extern const upb_msglayout envoy_api_v2_DiscoveryResponse_msginit;
extern const upb_msglayout envoy_api_v2_DeltaDiscoveryRequest_msginit;
extern const upb_msglayout envoy_api_v2_DeltaDiscoveryRequest_InitialResourceVersionsEntry_msginit;
extern const upb_msglayout envoy_api_v2_DeltaDiscoveryResponse_msginit;
extern const upb_msglayout envoy_api_v2_Resource_msginit;

static const upb_msglayout *layouts[6] = {
  &envoy_api_v2_DiscoveryRequest_msginit,
  &envoy_api_v2_DiscoveryResponse_msginit,
  &envoy_api_v2_DeltaDiscoveryRequest_msginit,
  &envoy_api_v2_DeltaDiscoveryRequest_InitialResourceVersionsEntry_msginit,
  &envoy_api_v2_DeltaDiscoveryResponse_msginit,
  &envoy_api_v2_Resource_msginit,
};

static const char descriptor[1567] =
  "\n\034envoy/api/v2/discovery.proto\022\014envoy.ap"
  "i.v2\032\034envoy/api/v2/core/base.proto\032\031goog"
  "le/protobuf/any.proto\032\027google/rpc/status"
  ".proto\"\202\002\n\020DiscoveryRequest\022!\n\014version_i"
  "nfo\030\001 \001(\tR\013versionInfo\022+\n\004node\030\002 \001(\0132\027.e"
  "nvoy.api.v2.core.NodeR\004node\022%\n\016resource_"
  "names\030\003 \003(\tR\rresourceNames\022\031\n\010type_url\030\004"
  " \001(\tR\007typeUrl\022%\n\016response_nonce\030\005 \001(\tR\rr"
  "esponseNonce\0225\n\014error_detail\030\006 \001(\0132\022.goo"
  "gle.rpc.StatusR\013errorDetail\"\371\001\n\021Discover"
  "yResponse\022!\n\014version_info\030\001 \001(\tR\013version"
  "Info\0222\n\tresources\030\002 \003(\0132\024.google.protobu"
  "f.AnyR\tresources\022\026\n\006canary\030\003 \001(\010R\006canary"
  "\022\031\n\010type_url\030\004 \001(\tR\007typeUrl\022\024\n\005nonce\030\005 \001"
  "(\tR\005nonce\022D\n\rcontrol_plane\030\006 \001(\0132\037.envoy"
  ".api.v2.core.ControlPlaneR\014controlPlane\""
  "\377\003\n\025DeltaDiscoveryRequest\022+\n\004node\030\001 \001(\0132"
  "\027.envoy.api.v2.core.NodeR\004node\022\031\n\010type_u"
  "rl\030\002 \001(\tR\007typeUrl\0228\n\030resource_names_subs"
  "cribe\030\003 \003(\tR\026resourceNamesSubscribe\022<\n\032r"
  "esource_names_unsubscribe\030\004 \003(\tR\030resourc"
  "eNamesUnsubscribe\022|\n\031initial_resource_ve"
  "rsions\030\005 \003(\0132@.envoy.api.v2.DeltaDiscove"
  "ryRequest.InitialResourceVersionsEntryR\027"
  "initialResourceVersions\022%\n\016response_nonc"
  "e\030\006 \001(\tR\rresponseNonce\0225\n\014error_detail\030\007"
  " \001(\0132\022.google.rpc.StatusR\013errorDetail\032J\n"
  "\034InitialResourceVersionsEntry\022\020\n\003key\030\001 \001"
  "(\tR\003key\022\024\n\005value\030\002 \001(\tR\005value:\0028\001\"\334\001\n\026De"
  "ltaDiscoveryResponse\022.\n\023system_version_i"
  "nfo\030\001 \001(\tR\021systemVersionInfo\0224\n\tresource"
  "s\030\002 \003(\0132\026.envoy.api.v2.ResourceR\tresourc"
  "es\022\031\n\010type_url\030\004 \001(\tR\007typeUrl\022+\n\021removed"
  "_resources\030\006 \003(\tR\020removedResources\022\024\n\005no"
  "nce\030\005 \001(\tR\005nonce\"\204\001\n\010Resource\022\022\n\004name\030\003 "
  "\001(\tR\004name\022\030\n\007aliases\030\004 \003(\tR\007aliases\022\030\n\007v"
  "ersion\030\001 \001(\tR\007version\0220\n\010resource\030\002 \001(\0132"
  "\024.google.protobuf.AnyR\010resourceB.\n\032io.en"
  "voyproxy.envoy.api.v2B\016DiscoveryProtoP\001b"
  "\006proto3";

static upb_def_init *deps[4] = {
  &envoy_api_v2_core_base_proto_upbdefinit,
  &google_protobuf_any_proto_upbdefinit,
  &google_rpc_status_proto_upbdefinit,
  NULL
};

upb_def_init envoy_api_v2_discovery_proto_upbdefinit = {
  deps,
  layouts,
  "envoy/api/v2/discovery.proto",
  UPB_STRVIEW_INIT(descriptor, 1567)
};
