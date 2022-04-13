/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     src/proto/grpc/gcp/handshaker.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include <stddef.h>
#include "upb/msg_internal.h"
#include "src/proto/grpc/gcp/handshaker.upb.h"
#include "src/proto/grpc/gcp/transport_security_common.upb.h"

#include "upb/port_def.inc"

static const upb_MiniTable_Field grpc_gcp_Endpoint__fields[3] = {
  {1, UPB_SIZE(8, 8), UPB_SIZE(0, 0), kUpb_NoSub, 9, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(0, 0), UPB_SIZE(0, 0), kUpb_NoSub, 5, kUpb_FieldMode_Scalar | (kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)},
  {3, UPB_SIZE(4, 4), UPB_SIZE(0, 0), kUpb_NoSub, 5, kUpb_FieldMode_Scalar | (kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_Endpoint_msginit = {
  NULL,
  &grpc_gcp_Endpoint__fields[0],
  UPB_SIZE(16, 24), 3, kUpb_ExtMode_NonExtendable, 3, 255, 0,
};

static const upb_MiniTable_Sub grpc_gcp_Identity_submsgs[1] = {
  {.submsg = &grpc_gcp_Identity_AttributesEntry_msginit},
};

static const upb_MiniTable_Field grpc_gcp_Identity__fields[3] = {
  {1, UPB_SIZE(4, 8), UPB_SIZE(-1, -1), kUpb_NoSub, 9, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(4, 8), UPB_SIZE(-1, -1), kUpb_NoSub, 9, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
  {3, UPB_SIZE(12, 24), UPB_SIZE(0, 0), 0, 11, kUpb_FieldMode_Map | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_Identity_msginit = {
  &grpc_gcp_Identity_submsgs[0],
  &grpc_gcp_Identity__fields[0],
  UPB_SIZE(16, 40), 3, kUpb_ExtMode_NonExtendable, 3, 255, 0,
};

static const upb_MiniTable_Field grpc_gcp_Identity_AttributesEntry__fields[2] = {
  {1, UPB_SIZE(0, 0), UPB_SIZE(0, 0), kUpb_NoSub, 12, kUpb_FieldMode_Scalar | (kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(8, 16), UPB_SIZE(0, 0), kUpb_NoSub, 12, kUpb_FieldMode_Scalar | (kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_Identity_AttributesEntry_msginit = {
  NULL,
  &grpc_gcp_Identity_AttributesEntry__fields[0],
  UPB_SIZE(16, 32), 2, kUpb_ExtMode_NonExtendable, 2, 255, 0,
};

static const upb_MiniTable_Sub grpc_gcp_StartClientHandshakeReq_submsgs[5] = {
  {.submsg = &grpc_gcp_Identity_msginit},
  {.submsg = &grpc_gcp_Identity_msginit},
  {.submsg = &grpc_gcp_Endpoint_msginit},
  {.submsg = &grpc_gcp_Endpoint_msginit},
  {.submsg = &grpc_gcp_RpcProtocolVersions_msginit},
};

static const upb_MiniTable_Field grpc_gcp_StartClientHandshakeReq__fields[10] = {
  {1, UPB_SIZE(4, 4), UPB_SIZE(0, 0), kUpb_NoSub, 5, kUpb_FieldMode_Scalar | (kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(12, 16), UPB_SIZE(0, 0), kUpb_NoSub, 9, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {3, UPB_SIZE(16, 24), UPB_SIZE(0, 0), kUpb_NoSub, 9, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {4, UPB_SIZE(20, 32), UPB_SIZE(0, 0), 0, 11, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {5, UPB_SIZE(24, 40), UPB_SIZE(1, 1), 1, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {6, UPB_SIZE(28, 48), UPB_SIZE(2, 2), 2, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {7, UPB_SIZE(32, 56), UPB_SIZE(3, 3), 3, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {8, UPB_SIZE(36, 64), UPB_SIZE(0, 0), kUpb_NoSub, 9, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
  {9, UPB_SIZE(44, 80), UPB_SIZE(4, 4), 4, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {10, UPB_SIZE(8, 8), UPB_SIZE(0, 0), kUpb_NoSub, 13, kUpb_FieldMode_Scalar | (kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_StartClientHandshakeReq_msginit = {
  &grpc_gcp_StartClientHandshakeReq_submsgs[0],
  &grpc_gcp_StartClientHandshakeReq__fields[0],
  UPB_SIZE(48, 88), 10, kUpb_ExtMode_NonExtendable, 10, 255, 0,
};

static const upb_MiniTable_Sub grpc_gcp_ServerHandshakeParameters_submsgs[1] = {
  {.submsg = &grpc_gcp_Identity_msginit},
};

static const upb_MiniTable_Field grpc_gcp_ServerHandshakeParameters__fields[2] = {
  {1, UPB_SIZE(0, 0), UPB_SIZE(0, 0), kUpb_NoSub, 9, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(4, 8), UPB_SIZE(0, 0), 0, 11, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_ServerHandshakeParameters_msginit = {
  &grpc_gcp_ServerHandshakeParameters_submsgs[0],
  &grpc_gcp_ServerHandshakeParameters__fields[0],
  UPB_SIZE(8, 24), 2, kUpb_ExtMode_NonExtendable, 2, 255, 0,
};

static const upb_MiniTable_Sub grpc_gcp_StartServerHandshakeReq_submsgs[4] = {
  {.submsg = &grpc_gcp_StartServerHandshakeReq_HandshakeParametersEntry_msginit},
  {.submsg = &grpc_gcp_Endpoint_msginit},
  {.submsg = &grpc_gcp_Endpoint_msginit},
  {.submsg = &grpc_gcp_RpcProtocolVersions_msginit},
};

static const upb_MiniTable_Field grpc_gcp_StartServerHandshakeReq__fields[7] = {
  {1, UPB_SIZE(8, 8), UPB_SIZE(0, 0), kUpb_NoSub, 9, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(12, 16), UPB_SIZE(0, 0), 0, 11, kUpb_FieldMode_Map | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {3, UPB_SIZE(16, 24), UPB_SIZE(0, 0), kUpb_NoSub, 12, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
  {4, UPB_SIZE(24, 40), UPB_SIZE(1, 1), 1, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {5, UPB_SIZE(28, 48), UPB_SIZE(2, 2), 2, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {6, UPB_SIZE(32, 56), UPB_SIZE(3, 3), 3, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {7, UPB_SIZE(4, 4), UPB_SIZE(0, 0), kUpb_NoSub, 13, kUpb_FieldMode_Scalar | (kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_StartServerHandshakeReq_msginit = {
  &grpc_gcp_StartServerHandshakeReq_submsgs[0],
  &grpc_gcp_StartServerHandshakeReq__fields[0],
  UPB_SIZE(36, 72), 7, kUpb_ExtMode_NonExtendable, 7, 255, 0,
};

static const upb_MiniTable_Sub grpc_gcp_StartServerHandshakeReq_HandshakeParametersEntry_submsgs[1] = {
  {.submsg = &grpc_gcp_ServerHandshakeParameters_msginit},
};

static const upb_MiniTable_Field grpc_gcp_StartServerHandshakeReq_HandshakeParametersEntry__fields[2] = {
  {1, UPB_SIZE(0, 0), UPB_SIZE(0, 0), kUpb_NoSub, 5, kUpb_FieldMode_Scalar | (kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(8, 16), UPB_SIZE(0, 0), 0, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_StartServerHandshakeReq_HandshakeParametersEntry_msginit = {
  &grpc_gcp_StartServerHandshakeReq_HandshakeParametersEntry_submsgs[0],
  &grpc_gcp_StartServerHandshakeReq_HandshakeParametersEntry__fields[0],
  UPB_SIZE(16, 32), 2, kUpb_ExtMode_NonExtendable, 2, 255, 0,
};

static const upb_MiniTable_Field grpc_gcp_NextHandshakeMessageReq__fields[1] = {
  {1, UPB_SIZE(0, 0), UPB_SIZE(0, 0), kUpb_NoSub, 12, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_NextHandshakeMessageReq_msginit = {
  NULL,
  &grpc_gcp_NextHandshakeMessageReq__fields[0],
  UPB_SIZE(8, 24), 1, kUpb_ExtMode_NonExtendable, 1, 255, 0,
};

static const upb_MiniTable_Sub grpc_gcp_HandshakerReq_submsgs[3] = {
  {.submsg = &grpc_gcp_StartClientHandshakeReq_msginit},
  {.submsg = &grpc_gcp_StartServerHandshakeReq_msginit},
  {.submsg = &grpc_gcp_NextHandshakeMessageReq_msginit},
};

static const upb_MiniTable_Field grpc_gcp_HandshakerReq__fields[3] = {
  {1, UPB_SIZE(4, 8), UPB_SIZE(-1, -1), 0, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(4, 8), UPB_SIZE(-1, -1), 1, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {3, UPB_SIZE(4, 8), UPB_SIZE(-1, -1), 2, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_HandshakerReq_msginit = {
  &grpc_gcp_HandshakerReq_submsgs[0],
  &grpc_gcp_HandshakerReq__fields[0],
  UPB_SIZE(8, 24), 3, kUpb_ExtMode_NonExtendable, 3, 255, 0,
};

static const upb_MiniTable_Sub grpc_gcp_HandshakerResult_submsgs[3] = {
  {.submsg = &grpc_gcp_Identity_msginit},
  {.submsg = &grpc_gcp_Identity_msginit},
  {.submsg = &grpc_gcp_RpcProtocolVersions_msginit},
};

static const upb_MiniTable_Field grpc_gcp_HandshakerResult__fields[8] = {
  {1, UPB_SIZE(8, 8), UPB_SIZE(0, 0), kUpb_NoSub, 9, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(16, 24), UPB_SIZE(0, 0), kUpb_NoSub, 9, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
  {3, UPB_SIZE(24, 40), UPB_SIZE(0, 0), kUpb_NoSub, 12, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
  {4, UPB_SIZE(32, 56), UPB_SIZE(1, 1), 0, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {5, UPB_SIZE(36, 64), UPB_SIZE(2, 2), 1, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {6, UPB_SIZE(1, 1), UPB_SIZE(0, 0), kUpb_NoSub, 8, kUpb_FieldMode_Scalar | (kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)},
  {7, UPB_SIZE(40, 72), UPB_SIZE(3, 3), 2, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {8, UPB_SIZE(4, 4), UPB_SIZE(0, 0), kUpb_NoSub, 13, kUpb_FieldMode_Scalar | (kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_HandshakerResult_msginit = {
  &grpc_gcp_HandshakerResult_submsgs[0],
  &grpc_gcp_HandshakerResult__fields[0],
  UPB_SIZE(44, 88), 8, kUpb_ExtMode_NonExtendable, 8, 255, 0,
};

static const upb_MiniTable_Field grpc_gcp_HandshakerStatus__fields[2] = {
  {1, UPB_SIZE(0, 0), UPB_SIZE(0, 0), kUpb_NoSub, 13, kUpb_FieldMode_Scalar | (kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(4, 8), UPB_SIZE(0, 0), kUpb_NoSub, 9, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_HandshakerStatus_msginit = {
  NULL,
  &grpc_gcp_HandshakerStatus__fields[0],
  UPB_SIZE(12, 24), 2, kUpb_ExtMode_NonExtendable, 2, 255, 0,
};

static const upb_MiniTable_Sub grpc_gcp_HandshakerResp_submsgs[2] = {
  {.submsg = &grpc_gcp_HandshakerResult_msginit},
  {.submsg = &grpc_gcp_HandshakerStatus_msginit},
};

static const upb_MiniTable_Field grpc_gcp_HandshakerResp__fields[4] = {
  {1, UPB_SIZE(8, 8), UPB_SIZE(0, 0), kUpb_NoSub, 12, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(4, 4), UPB_SIZE(0, 0), kUpb_NoSub, 13, kUpb_FieldMode_Scalar | (kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)},
  {3, UPB_SIZE(16, 24), UPB_SIZE(1, 1), 0, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {4, UPB_SIZE(20, 32), UPB_SIZE(2, 2), 1, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
};

const upb_MiniTable grpc_gcp_HandshakerResp_msginit = {
  &grpc_gcp_HandshakerResp_submsgs[0],
  &grpc_gcp_HandshakerResp__fields[0],
  UPB_SIZE(24, 40), 4, kUpb_ExtMode_NonExtendable, 4, 255, 0,
};

static const upb_MiniTable *messages_layout[12] = {
  &grpc_gcp_Endpoint_msginit,
  &grpc_gcp_Identity_msginit,
  &grpc_gcp_Identity_AttributesEntry_msginit,
  &grpc_gcp_StartClientHandshakeReq_msginit,
  &grpc_gcp_ServerHandshakeParameters_msginit,
  &grpc_gcp_StartServerHandshakeReq_msginit,
  &grpc_gcp_StartServerHandshakeReq_HandshakeParametersEntry_msginit,
  &grpc_gcp_NextHandshakeMessageReq_msginit,
  &grpc_gcp_HandshakerReq_msginit,
  &grpc_gcp_HandshakerResult_msginit,
  &grpc_gcp_HandshakerStatus_msginit,
  &grpc_gcp_HandshakerResp_msginit,
};

const upb_MiniTable_File src_proto_grpc_gcp_handshaker_proto_upb_file_layout = {
  messages_layout,
  NULL,
  NULL,
  12,
  0,
  0,
};

#include "upb/port_undef.inc"

