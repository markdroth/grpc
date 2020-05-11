/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/api/v2/listener.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init envoy_api_v2_core_address_proto_upbdefinit;
extern upb_def_init envoy_api_v2_core_base_proto_upbdefinit;
extern upb_def_init envoy_api_v2_core_socket_option_proto_upbdefinit;
extern upb_def_init envoy_api_v2_listener_listener_components_proto_upbdefinit;
extern upb_def_init envoy_api_v2_listener_udp_listener_config_proto_upbdefinit;
extern upb_def_init envoy_config_filter_accesslog_v2_accesslog_proto_upbdefinit;
extern upb_def_init envoy_config_listener_v2_api_listener_proto_upbdefinit;
extern upb_def_init google_api_annotations_proto_upbdefinit;
extern upb_def_init google_protobuf_duration_proto_upbdefinit;
extern upb_def_init google_protobuf_wrappers_proto_upbdefinit;
extern upb_def_init udpa_annotations_migrate_proto_upbdefinit;
extern upb_def_init udpa_annotations_status_proto_upbdefinit;
extern upb_def_init validate_validate_proto_upbdefinit;
extern const upb_msglayout envoy_api_v2_Listener_msginit;
extern const upb_msglayout envoy_api_v2_Listener_DeprecatedV1_msginit;
extern const upb_msglayout envoy_api_v2_Listener_ConnectionBalanceConfig_msginit;
extern const upb_msglayout envoy_api_v2_Listener_ConnectionBalanceConfig_ExactBalance_msginit;

static const upb_msglayout *layouts[4] = {
  &envoy_api_v2_Listener_msginit,
  &envoy_api_v2_Listener_DeprecatedV1_msginit,
  &envoy_api_v2_Listener_ConnectionBalanceConfig_msginit,
  &envoy_api_v2_Listener_ConnectionBalanceConfig_ExactBalance_msginit,
};

static const char descriptor[2439] =
  "\n\033envoy/api/v2/listener.proto\022\014envoy.api"
  ".v2\032\037envoy/api/v2/core/address.proto\032\034en"
  "voy/api/v2/core/base.proto\032%envoy/api/v2"
  "/core/socket_option.proto\032/envoy/api/v2/"
  "listener/listener_components.proto\032/envo"
  "y/api/v2/listener/udp_listener_config.pr"
  "oto\0320envoy/config/filter/accesslog/v2/ac"
  "cesslog.proto\032+envoy/config/listener/v2/"
  "api_listener.proto\032\034google/api/annotatio"
  "ns.proto\032\036google/protobuf/duration.proto"
  "\032\036google/protobuf/wrappers.proto\032\036udpa/a"
  "nnotations/migrate.proto\032\035udpa/annotatio"
  "ns/status.proto\032\027validate/validate.proto"
  "\"\235\016\n\010Listener\022\022\n\004name\030\001 \001(\tR\004name\022>\n\007add"
  "ress\030\002 \001(\0132\032.envoy.api.v2.core.AddressB\010"
  "\372B\005\212\001\002\020\001R\007address\022G\n\rfilter_chains\030\003 \003(\013"
  "2\".envoy.api.v2.listener.FilterChainR\014fi"
  "lterChains\022H\n\020use_original_dst\030\004 \001(\0132\032.g"
  "oogle.protobuf.BoolValueB\002\030\001R\016useOrigina"
  "lDst\022f\n!per_connection_buffer_limit_byte"
  "s\030\005 \001(\0132\034.google.protobuf.UInt32ValueR\035p"
  "erConnectionBufferLimitBytes\0227\n\010metadata"
  "\030\006 \001(\0132\033.envoy.api.v2.core.MetadataR\010met"
  "adata\022H\n\rdeprecated_v1\030\007 \001(\0132#.envoy.api"
  ".v2.Listener.DeprecatedV1R\014deprecatedV1\022"
  "\?\n\ndrain_type\030\010 \001(\0162 .envoy.api.v2.Liste"
  "ner.DrainTypeR\tdrainType\022P\n\020listener_fil"
  "ters\030\t \003(\0132%.envoy.api.v2.listener.Liste"
  "nerFilterR\017listenerFilters\022S\n\030listener_f"
  "ilters_timeout\030\017 \001(\0132\031.google.protobuf.D"
  "urationR\026listenerFiltersTimeout\022N\n$conti"
  "nue_on_listener_filters_timeout\030\021 \001(\010R c"
  "ontinueOnListenerFiltersTimeout\022<\n\013trans"
  "parent\030\n \001(\0132\032.google.protobuf.BoolValue"
  "R\013transparent\0226\n\010freebind\030\013 \001(\0132\032.google"
  ".protobuf.BoolValueR\010freebind\022F\n\016socket_"
  "options\030\r \003(\0132\037.envoy.api.v2.core.Socket"
  "OptionR\rsocketOptions\022X\n\032tcp_fast_open_q"
  "ueue_length\030\014 \001(\0132\034.google.protobuf.UInt"
  "32ValueR\026tcpFastOpenQueueLength\022P\n\021traff"
  "ic_direction\030\020 \001(\0162#.envoy.api.v2.core.T"
  "rafficDirectionR\020trafficDirection\022X\n\023udp"
  "_listener_config\030\022 \001(\0132(.envoy.api.v2.li"
  "stener.UdpListenerConfigR\021udpListenerCon"
  "fig\022H\n\014api_listener\030\023 \001(\0132%.envoy.config"
  ".listener.v2.ApiListenerR\013apiListener\022j\n"
  "\031connection_balance_config\030\024 \001(\0132..envoy"
  ".api.v2.Listener.ConnectionBalanceConfig"
  "R\027connectionBalanceConfig\022\035\n\nreuse_port\030"
  "\025 \001(\010R\treusePort\022J\n\naccess_log\030\026 \003(\0132+.e"
  "nvoy.config.filter.accesslog.v2.AccessLo"
  "gR\taccessLog\032L\n\014DeprecatedV1\022<\n\014bind_to_"
  "port\030\001 \001(\0132\032.google.protobuf.BoolValueR\n"
  "bindToPort\032\242\001\n\027ConnectionBalanceConfig\022b"
  "\n\rexact_balance\030\001 \001(\0132;.envoy.api.v2.Lis"
  "tener.ConnectionBalanceConfig.ExactBalan"
  "ceH\000R\014exactBalance\032\016\n\014ExactBalanceB\023\n\014ba"
  "lance_type\022\003\370B\001\")\n\tDrainType\022\013\n\007DEFAULT\020"
  "\000\022\017\n\013MODIFY_ONLY\020\001J\004\010\016\020\017BU\n\032io.envoyprox"
  "y.envoy.api.v2B\rListenerProtoP\001\362\230\376\217\005\032\022\030e"
  "nvoy.config.listener.v3\272\200\310\321\006\002\020\001b\006proto3";

static upb_def_init *deps[14] = {
  &envoy_api_v2_core_address_proto_upbdefinit,
  &envoy_api_v2_core_base_proto_upbdefinit,
  &envoy_api_v2_core_socket_option_proto_upbdefinit,
  &envoy_api_v2_listener_listener_components_proto_upbdefinit,
  &envoy_api_v2_listener_udp_listener_config_proto_upbdefinit,
  &envoy_config_filter_accesslog_v2_accesslog_proto_upbdefinit,
  &envoy_config_listener_v2_api_listener_proto_upbdefinit,
  &google_api_annotations_proto_upbdefinit,
  &google_protobuf_duration_proto_upbdefinit,
  &google_protobuf_wrappers_proto_upbdefinit,
  &udpa_annotations_migrate_proto_upbdefinit,
  &udpa_annotations_status_proto_upbdefinit,
  &validate_validate_proto_upbdefinit,
  NULL
};

upb_def_init envoy_api_v2_listener_proto_upbdefinit = {
  deps,
  layouts,
  "envoy/api/v2/listener.proto",
  UPB_STRVIEW_INIT(descriptor, 2439)
};
