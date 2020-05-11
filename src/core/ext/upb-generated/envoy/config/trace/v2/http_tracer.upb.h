/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/config/trace/v2/http_tracer.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#ifndef ENVOY_CONFIG_TRACE_V2_HTTP_TRACER_PROTO_UPB_H_
#define ENVOY_CONFIG_TRACE_V2_HTTP_TRACER_PROTO_UPB_H_

#include "upb/msg.h"
#include "upb/decode.h"
#include "upb/encode.h"

#include "upb/port_def.inc"

#ifdef __cplusplus
extern "C" {
#endif

struct envoy_config_trace_v2_Tracing;
struct envoy_config_trace_v2_Tracing_Http;
typedef struct envoy_config_trace_v2_Tracing envoy_config_trace_v2_Tracing;
typedef struct envoy_config_trace_v2_Tracing_Http envoy_config_trace_v2_Tracing_Http;
extern const upb_msglayout envoy_config_trace_v2_Tracing_msginit;
extern const upb_msglayout envoy_config_trace_v2_Tracing_Http_msginit;
struct google_protobuf_Any;
struct google_protobuf_Struct;
extern const upb_msglayout google_protobuf_Any_msginit;
extern const upb_msglayout google_protobuf_Struct_msginit;


/* envoy.config.trace.v2.Tracing */

UPB_INLINE envoy_config_trace_v2_Tracing *envoy_config_trace_v2_Tracing_new(upb_arena *arena) {
  return (envoy_config_trace_v2_Tracing *)_upb_msg_new(&envoy_config_trace_v2_Tracing_msginit, arena);
}
UPB_INLINE envoy_config_trace_v2_Tracing *envoy_config_trace_v2_Tracing_parse(const char *buf, size_t size,
                        upb_arena *arena) {
  envoy_config_trace_v2_Tracing *ret = envoy_config_trace_v2_Tracing_new(arena);
  return (ret && upb_decode(buf, size, ret, &envoy_config_trace_v2_Tracing_msginit, arena)) ? ret : NULL;
}
UPB_INLINE char *envoy_config_trace_v2_Tracing_serialize(const envoy_config_trace_v2_Tracing *msg, upb_arena *arena, size_t *len) {
  return upb_encode(msg, &envoy_config_trace_v2_Tracing_msginit, arena, len);
}

UPB_INLINE const envoy_config_trace_v2_Tracing_Http* envoy_config_trace_v2_Tracing_http(const envoy_config_trace_v2_Tracing *msg) { return UPB_FIELD_AT(msg, const envoy_config_trace_v2_Tracing_Http*, UPB_SIZE(0, 0)); }

UPB_INLINE void envoy_config_trace_v2_Tracing_set_http(envoy_config_trace_v2_Tracing *msg, envoy_config_trace_v2_Tracing_Http* value) {
  UPB_FIELD_AT(msg, envoy_config_trace_v2_Tracing_Http*, UPB_SIZE(0, 0)) = value;
}
UPB_INLINE struct envoy_config_trace_v2_Tracing_Http* envoy_config_trace_v2_Tracing_mutable_http(envoy_config_trace_v2_Tracing *msg, upb_arena *arena) {
  struct envoy_config_trace_v2_Tracing_Http* sub = (struct envoy_config_trace_v2_Tracing_Http*)envoy_config_trace_v2_Tracing_http(msg);
  if (sub == NULL) {
    sub = (struct envoy_config_trace_v2_Tracing_Http*)_upb_msg_new(&envoy_config_trace_v2_Tracing_Http_msginit, arena);
    if (!sub) return NULL;
    envoy_config_trace_v2_Tracing_set_http(msg, sub);
  }
  return sub;
}

/* envoy.config.trace.v2.Tracing.Http */

UPB_INLINE envoy_config_trace_v2_Tracing_Http *envoy_config_trace_v2_Tracing_Http_new(upb_arena *arena) {
  return (envoy_config_trace_v2_Tracing_Http *)_upb_msg_new(&envoy_config_trace_v2_Tracing_Http_msginit, arena);
}
UPB_INLINE envoy_config_trace_v2_Tracing_Http *envoy_config_trace_v2_Tracing_Http_parse(const char *buf, size_t size,
                        upb_arena *arena) {
  envoy_config_trace_v2_Tracing_Http *ret = envoy_config_trace_v2_Tracing_Http_new(arena);
  return (ret && upb_decode(buf, size, ret, &envoy_config_trace_v2_Tracing_Http_msginit, arena)) ? ret : NULL;
}
UPB_INLINE char *envoy_config_trace_v2_Tracing_Http_serialize(const envoy_config_trace_v2_Tracing_Http *msg, upb_arena *arena, size_t *len) {
  return upb_encode(msg, &envoy_config_trace_v2_Tracing_Http_msginit, arena, len);
}

typedef enum {
  envoy_config_trace_v2_Tracing_Http_config_type_config = 2,
  envoy_config_trace_v2_Tracing_Http_config_type_typed_config = 3,
  envoy_config_trace_v2_Tracing_Http_config_type_NOT_SET = 0
} envoy_config_trace_v2_Tracing_Http_config_type_oneofcases;
UPB_INLINE envoy_config_trace_v2_Tracing_Http_config_type_oneofcases envoy_config_trace_v2_Tracing_Http_config_type_case(const envoy_config_trace_v2_Tracing_Http* msg) { return (envoy_config_trace_v2_Tracing_Http_config_type_oneofcases)UPB_FIELD_AT(msg, int32_t, UPB_SIZE(12, 24)); }

UPB_INLINE upb_strview envoy_config_trace_v2_Tracing_Http_name(const envoy_config_trace_v2_Tracing_Http *msg) { return UPB_FIELD_AT(msg, upb_strview, UPB_SIZE(0, 0)); }
UPB_INLINE bool envoy_config_trace_v2_Tracing_Http_has_config(const envoy_config_trace_v2_Tracing_Http *msg) { return _upb_has_oneof_field(msg, UPB_SIZE(12, 24), 2); }
UPB_INLINE const struct google_protobuf_Struct* envoy_config_trace_v2_Tracing_Http_config(const envoy_config_trace_v2_Tracing_Http *msg) { return UPB_READ_ONEOF(msg, const struct google_protobuf_Struct*, UPB_SIZE(8, 16), UPB_SIZE(12, 24), 2, NULL); }
UPB_INLINE bool envoy_config_trace_v2_Tracing_Http_has_typed_config(const envoy_config_trace_v2_Tracing_Http *msg) { return _upb_has_oneof_field(msg, UPB_SIZE(12, 24), 3); }
UPB_INLINE const struct google_protobuf_Any* envoy_config_trace_v2_Tracing_Http_typed_config(const envoy_config_trace_v2_Tracing_Http *msg) { return UPB_READ_ONEOF(msg, const struct google_protobuf_Any*, UPB_SIZE(8, 16), UPB_SIZE(12, 24), 3, NULL); }

UPB_INLINE void envoy_config_trace_v2_Tracing_Http_set_name(envoy_config_trace_v2_Tracing_Http *msg, upb_strview value) {
  UPB_FIELD_AT(msg, upb_strview, UPB_SIZE(0, 0)) = value;
}
UPB_INLINE void envoy_config_trace_v2_Tracing_Http_set_config(envoy_config_trace_v2_Tracing_Http *msg, struct google_protobuf_Struct* value) {
  UPB_WRITE_ONEOF(msg, struct google_protobuf_Struct*, UPB_SIZE(8, 16), value, UPB_SIZE(12, 24), 2);
}
UPB_INLINE struct google_protobuf_Struct* envoy_config_trace_v2_Tracing_Http_mutable_config(envoy_config_trace_v2_Tracing_Http *msg, upb_arena *arena) {
  struct google_protobuf_Struct* sub = (struct google_protobuf_Struct*)envoy_config_trace_v2_Tracing_Http_config(msg);
  if (sub == NULL) {
    sub = (struct google_protobuf_Struct*)_upb_msg_new(&google_protobuf_Struct_msginit, arena);
    if (!sub) return NULL;
    envoy_config_trace_v2_Tracing_Http_set_config(msg, sub);
  }
  return sub;
}
UPB_INLINE void envoy_config_trace_v2_Tracing_Http_set_typed_config(envoy_config_trace_v2_Tracing_Http *msg, struct google_protobuf_Any* value) {
  UPB_WRITE_ONEOF(msg, struct google_protobuf_Any*, UPB_SIZE(8, 16), value, UPB_SIZE(12, 24), 3);
}
UPB_INLINE struct google_protobuf_Any* envoy_config_trace_v2_Tracing_Http_mutable_typed_config(envoy_config_trace_v2_Tracing_Http *msg, upb_arena *arena) {
  struct google_protobuf_Any* sub = (struct google_protobuf_Any*)envoy_config_trace_v2_Tracing_Http_typed_config(msg);
  if (sub == NULL) {
    sub = (struct google_protobuf_Any*)_upb_msg_new(&google_protobuf_Any_msginit, arena);
    if (!sub) return NULL;
    envoy_config_trace_v2_Tracing_Http_set_typed_config(msg, sub);
  }
  return sub;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#include "upb/port_undef.inc"

#endif  /* ENVOY_CONFIG_TRACE_V2_HTTP_TRACER_PROTO_UPB_H_ */
