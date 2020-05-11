/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/api/v2/listener/listener_components.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init envoy_api_v2_auth_tls_proto_upbdefinit;
extern upb_def_init envoy_api_v2_core_address_proto_upbdefinit;
extern upb_def_init envoy_api_v2_core_base_proto_upbdefinit;
extern upb_def_init envoy_type_range_proto_upbdefinit;
extern upb_def_init google_protobuf_any_proto_upbdefinit;
extern upb_def_init google_protobuf_struct_proto_upbdefinit;
extern upb_def_init google_protobuf_wrappers_proto_upbdefinit;
extern upb_def_init udpa_annotations_migrate_proto_upbdefinit;
extern upb_def_init udpa_annotations_status_proto_upbdefinit;
extern upb_def_init validate_validate_proto_upbdefinit;
extern const upb_msglayout envoy_api_v2_listener_Filter_msginit;
extern const upb_msglayout envoy_api_v2_listener_FilterChainMatch_msginit;
extern const upb_msglayout envoy_api_v2_listener_FilterChain_msginit;
extern const upb_msglayout envoy_api_v2_listener_ListenerFilterChainMatchPredicate_msginit;
extern const upb_msglayout envoy_api_v2_listener_ListenerFilterChainMatchPredicate_MatchSet_msginit;
extern const upb_msglayout envoy_api_v2_listener_ListenerFilter_msginit;

static const upb_msglayout *layouts[6] = {
  &envoy_api_v2_listener_Filter_msginit,
  &envoy_api_v2_listener_FilterChainMatch_msginit,
  &envoy_api_v2_listener_FilterChain_msginit,
  &envoy_api_v2_listener_ListenerFilterChainMatchPredicate_msginit,
  &envoy_api_v2_listener_ListenerFilterChainMatchPredicate_MatchSet_msginit,
  &envoy_api_v2_listener_ListenerFilter_msginit,
};

static const char descriptor[2741] =
  "\n/envoy/api/v2/listener/listener_compone"
  "nts.proto\022\025envoy.api.v2.listener\032\033envoy/"
  "api/v2/auth/tls.proto\032\037envoy/api/v2/core"
  "/address.proto\032\034envoy/api/v2/core/base.p"
  "roto\032\026envoy/type/range.proto\032\031google/pro"
  "tobuf/any.proto\032\034google/protobuf/struct."
  "proto\032\036google/protobuf/wrappers.proto\032\036u"
  "dpa/annotations/migrate.proto\032\035udpa/anno"
  "tations/status.proto\032\027validate/validate."
  "proto\"\254\001\n\006Filter\022\033\n\004name\030\001 \001(\tB\007\372B\004r\002 \001R"
  "\004name\0225\n\006config\030\002 \001(\0132\027.google.protobuf."
  "StructB\002\030\001H\000R\006config\0229\n\014typed_config\030\004 \001"
  "(\0132\024.google.protobuf.AnyH\000R\013typedConfigB"
  "\r\n\013config_typeJ\004\010\003\020\004\"\341\005\n\020FilterChainMatc"
  "h\022T\n\020destination_port\030\010 \001(\0132\034.google.pro"
  "tobuf.UInt32ValueB\013\372B\010*\006\030\377\377\003(\001R\017destinat"
  "ionPort\022A\n\rprefix_ranges\030\003 \003(\0132\034.envoy.a"
  "pi.v2.core.CidrRangeR\014prefixRanges\022%\n\016ad"
  "dress_suffix\030\004 \001(\tR\raddressSuffix\022;\n\nsuf"
  "fix_len\030\005 \001(\0132\034.google.protobuf.UInt32Va"
  "lueR\tsuffixLen\022g\n\013source_type\030\014 \001(\0162<.en"
  "voy.api.v2.listener.FilterChainMatch.Con"
  "nectionSourceTypeB\010\372B\005\202\001\002\020\001R\nsourceType\022"
  "N\n\024source_prefix_ranges\030\006 \003(\0132\034.envoy.ap"
  "i.v2.core.CidrRangeR\022sourcePrefixRanges\022"
  "3\n\014source_ports\030\007 \003(\rB\020\372B\r\222\001\n\"\010*\006\030\377\377\003(\001R"
  "\013sourcePorts\022!\n\014server_names\030\013 \003(\tR\013serv"
  "erNames\022-\n\022transport_protocol\030\t \001(\tR\021tra"
  "nsportProtocol\0223\n\025application_protocols\030"
  "\n \003(\tR\024applicationProtocols\"U\n\024Connectio"
  "nSourceType\022\007\n\003ANY\020\000\022&\n\005LOCAL\020\001\032\033\362\230\376\217\005\025\n"
  "\023SAME_IP_OR_LOOPBACK\022\014\n\010EXTERNAL\020\002J\004\010\001\020\002"
  "\"\313\003\n\013FilterChain\022U\n\022filter_chain_match\030\001"
  " \001(\0132\'.envoy.api.v2.listener.FilterChain"
  "MatchR\020filterChainMatch\022L\n\013tls_context\030\002"
  " \001(\0132\'.envoy.api.v2.auth.DownstreamTlsCo"
  "ntextB\002\030\001R\ntlsContext\0227\n\007filters\030\003 \003(\0132\035"
  ".envoy.api.v2.listener.FilterR\007filters\022B"
  "\n\017use_proxy_proto\030\004 \001(\0132\032.google.protobu"
  "f.BoolValueR\ruseProxyProto\0227\n\010metadata\030\005"
  " \001(\0132\033.envoy.api.v2.core.MetadataR\010metad"
  "ata\022M\n\020transport_socket\030\006 \001(\0132\".envoy.ap"
  "i.v2.core.TransportSocketR\017transportSock"
  "et\022\022\n\004name\030\007 \001(\tR\004name\"\251\004\n!ListenerFilte"
  "rChainMatchPredicate\022^\n\010or_match\030\001 \001(\0132A"
  ".envoy.api.v2.listener.ListenerFilterCha"
  "inMatchPredicate.MatchSetH\000R\007orMatch\022`\n\t"
  "and_match\030\002 \001(\0132A.envoy.api.v2.listener."
  "ListenerFilterChainMatchPredicate.MatchS"
  "etH\000R\010andMatch\022W\n\tnot_match\030\003 \001(\01328.envo"
  "y.api.v2.listener.ListenerFilterChainMat"
  "chPredicateH\000R\010notMatch\022&\n\tany_match\030\004 \001"
  "(\010B\007\372B\004j\002\010\001H\000R\010anyMatch\022N\n\026destination_p"
  "ort_range\030\005 \001(\0132\026.envoy.type.Int32RangeH"
  "\000R\024destinationPortRange\032d\n\010MatchSet\022X\n\005r"
  "ules\030\001 \003(\01328.envoy.api.v2.listener.Liste"
  "nerFilterChainMatchPredicateB\010\372B\005\222\001\002\010\002R\005"
  "rulesB\013\n\004rule\022\003\370B\001\"\221\002\n\016ListenerFilter\022\033\n"
  "\004name\030\001 \001(\tB\007\372B\004r\002 \001R\004name\0225\n\006config\030\002 \001"
  "(\0132\027.google.protobuf.StructB\002\030\001H\000R\006confi"
  "g\0229\n\014typed_config\030\003 \001(\0132\024.google.protobu"
  "f.AnyH\000R\013typedConfig\022a\n\017filter_disabled\030"
  "\004 \001(\01328.envoy.api.v2.listener.ListenerFi"
  "lterChainMatchPredicateR\016filterDisabledB"
  "\r\n\013config_typeB\234\001\n#io.envoyproxy.envoy.a"
  "pi.v2.listenerB\027ListenerComponentsProtoP"
  "\001\252\002\027Envoy.Api.V2.ListenerNS\352\002\027Envoy.Api."
  "V2.ListenerNS\362\230\376\217\005\032\022\030envoy.config.listen"
  "er.v3\272\200\310\321\006\002\020\001b\006proto3";

static upb_def_init *deps[11] = {
  &envoy_api_v2_auth_tls_proto_upbdefinit,
  &envoy_api_v2_core_address_proto_upbdefinit,
  &envoy_api_v2_core_base_proto_upbdefinit,
  &envoy_type_range_proto_upbdefinit,
  &google_protobuf_any_proto_upbdefinit,
  &google_protobuf_struct_proto_upbdefinit,
  &google_protobuf_wrappers_proto_upbdefinit,
  &udpa_annotations_migrate_proto_upbdefinit,
  &udpa_annotations_status_proto_upbdefinit,
  &validate_validate_proto_upbdefinit,
  NULL
};

upb_def_init envoy_api_v2_listener_listener_components_proto_upbdefinit = {
  deps,
  layouts,
  "envoy/api/v2/listener/listener_components.proto",
  UPB_STRVIEW_INIT(descriptor, 2741)
};
