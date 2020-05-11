/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/api/v2/auth/secret.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init envoy_api_v2_auth_common_proto_upbdefinit;
extern upb_def_init envoy_api_v2_core_base_proto_upbdefinit;
extern upb_def_init envoy_api_v2_core_config_source_proto_upbdefinit;
extern upb_def_init udpa_annotations_migrate_proto_upbdefinit;
extern upb_def_init udpa_annotations_sensitive_proto_upbdefinit;
extern upb_def_init udpa_annotations_status_proto_upbdefinit;
extern const upb_msglayout envoy_api_v2_auth_GenericSecret_msginit;
extern const upb_msglayout envoy_api_v2_auth_SdsSecretConfig_msginit;
extern const upb_msglayout envoy_api_v2_auth_Secret_msginit;

static const upb_msglayout *layouts[3] = {
  &envoy_api_v2_auth_GenericSecret_msginit,
  &envoy_api_v2_auth_SdsSecretConfig_msginit,
  &envoy_api_v2_auth_Secret_msginit,
};

static const char descriptor[928] =
  "\n\036envoy/api/v2/auth/secret.proto\022\021envoy."
  "api.v2.auth\032\036envoy/api/v2/auth/common.pr"
  "oto\032\034envoy/api/v2/core/base.proto\032%envoy"
  "/api/v2/core/config_source.proto\032\036udpa/a"
  "nnotations/migrate.proto\032 udpa/annotatio"
  "ns/sensitive.proto\032\035udpa/annotations/sta"
  "tus.proto\"N\n\rGenericSecret\022=\n\006secret\030\001 \001"
  "(\0132\035.envoy.api.v2.core.DataSourceB\006\270\267\213\244\002"
  "\001R\006secret\"e\n\017SdsSecretConfig\022\022\n\004name\030\001 \001"
  "(\tR\004name\022>\n\nsds_config\030\002 \001(\0132\037.envoy.api"
  ".v2.core.ConfigSourceR\tsdsConfig\"\372\002\n\006Sec"
  "ret\022\022\n\004name\030\001 \001(\tR\004name\022L\n\017tls_certifica"
  "te\030\002 \001(\0132!.envoy.api.v2.auth.TlsCertific"
  "ateH\000R\016tlsCertificate\022Y\n\023session_ticket_"
  "keys\030\003 \001(\0132\'.envoy.api.v2.auth.TlsSessio"
  "nTicketKeysH\000R\021sessionTicketKeys\022`\n\022vali"
  "dation_context\030\004 \001(\0132/.envoy.api.v2.auth"
  ".CertificateValidationContextH\000R\021validat"
  "ionContext\022I\n\016generic_secret\030\005 \001(\0132 .env"
  "oy.api.v2.auth.GenericSecretH\000R\rgenericS"
  "ecretB\006\n\004typeBi\n\037io.envoyproxy.envoy.api"
  ".v2.authB\013SecretProtoP\001\362\230\376\217\005+\022)envoy.ext"
  "ensions.transport_sockets.tls.v3\272\200\310\321\006\002\020\001"
  "b\006proto3";

static upb_def_init *deps[7] = {
  &envoy_api_v2_auth_common_proto_upbdefinit,
  &envoy_api_v2_core_base_proto_upbdefinit,
  &envoy_api_v2_core_config_source_proto_upbdefinit,
  &udpa_annotations_migrate_proto_upbdefinit,
  &udpa_annotations_sensitive_proto_upbdefinit,
  &udpa_annotations_status_proto_upbdefinit,
  NULL
};

upb_def_init envoy_api_v2_auth_secret_proto_upbdefinit = {
  deps,
  layouts,
  "envoy/api/v2/auth/secret.proto",
  UPB_STRVIEW_INIT(descriptor, 928)
};
