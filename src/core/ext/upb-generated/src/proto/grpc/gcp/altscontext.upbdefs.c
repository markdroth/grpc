/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     src/proto/grpc/gcp/altscontext.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init src_proto_grpc_gcp_transport_security_common_proto_upbdefinit;
extern const upb_msglayout grpc_gcp_AltsContext_msginit;
extern const upb_msglayout grpc_gcp_AltsContext_PeerAttributesEntry_msginit;

static const upb_msglayout *layouts[2] = {
  &grpc_gcp_AltsContext_msginit,
  &grpc_gcp_AltsContext_PeerAttributesEntry_msginit,
};

static const char descriptor[718] =
  "\n$src/proto/grpc/gcp/altscontext.proto\022\010"
  "grpc.gcp\0322src/proto/grpc/gcp/transport_s"
  "ecurity_common.proto\"\361\003\n\013AltsContext\0221\n\024"
  "application_protocol\030\001 \001(\tR\023applicationP"
  "rotocol\022\'\n\017record_protocol\030\002 \001(\tR\016record"
  "Protocol\022>\n\016security_level\030\003 \001(\0162\027.grpc."
  "gcp.SecurityLevelR\rsecurityLevel\0220\n\024peer"
  "_service_account\030\004 \001(\tR\022peerServiceAccou"
  "nt\0222\n\025local_service_account\030\005 \001(\tR\023local"
  "ServiceAccount\022I\n\021peer_rpc_versions\030\006 \001("
  "\0132\035.grpc.gcp.RpcProtocolVersionsR\017peerRp"
  "cVersions\022R\n\017peer_attributes\030\007 \003(\0132).grp"
  "c.gcp.AltsContext.PeerAttributesEntryR\016p"
  "eerAttributes\032A\n\023PeerAttributesEntry\022\020\n\003"
  "key\030\001 \001(\tR\003key\022\024\n\005value\030\002 \001(\tR\005value:\0028\001"
  "Bl\n\025io.grpc.alts.internalB\020AltsContextPr"
  "otoP\001Z\?google.golang.org/grpc/credential"
  "s/alts/internal/proto/grpc_gcpb\006proto3";

static upb_def_init *deps[2] = {
  &src_proto_grpc_gcp_transport_security_common_proto_upbdefinit,
  NULL
};

upb_def_init src_proto_grpc_gcp_altscontext_proto_upbdefinit = {
  deps,
  layouts,
  "src/proto/grpc/gcp/altscontext.proto",
  UPB_STRVIEW_INIT(descriptor, 718)
};
