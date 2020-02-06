/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     validate/validate.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include "upb/def.h"

extern upb_def_init google_protobuf_descriptor_proto_upbdefinit;
extern upb_def_init google_protobuf_duration_proto_upbdefinit;
extern upb_def_init google_protobuf_timestamp_proto_upbdefinit;
extern const upb_msglayout validate_FieldRules_msginit;
extern const upb_msglayout validate_FloatRules_msginit;
extern const upb_msglayout validate_DoubleRules_msginit;
extern const upb_msglayout validate_Int32Rules_msginit;
extern const upb_msglayout validate_Int64Rules_msginit;
extern const upb_msglayout validate_UInt32Rules_msginit;
extern const upb_msglayout validate_UInt64Rules_msginit;
extern const upb_msglayout validate_SInt32Rules_msginit;
extern const upb_msglayout validate_SInt64Rules_msginit;
extern const upb_msglayout validate_Fixed32Rules_msginit;
extern const upb_msglayout validate_Fixed64Rules_msginit;
extern const upb_msglayout validate_SFixed32Rules_msginit;
extern const upb_msglayout validate_SFixed64Rules_msginit;
extern const upb_msglayout validate_BoolRules_msginit;
extern const upb_msglayout validate_StringRules_msginit;
extern const upb_msglayout validate_BytesRules_msginit;
extern const upb_msglayout validate_EnumRules_msginit;
extern const upb_msglayout validate_MessageRules_msginit;
extern const upb_msglayout validate_RepeatedRules_msginit;
extern const upb_msglayout validate_MapRules_msginit;
extern const upb_msglayout validate_AnyRules_msginit;
extern const upb_msglayout validate_DurationRules_msginit;
extern const upb_msglayout validate_TimestampRules_msginit;

static const upb_msglayout *layouts[23] = {
  &validate_FieldRules_msginit,
  &validate_FloatRules_msginit,
  &validate_DoubleRules_msginit,
  &validate_Int32Rules_msginit,
  &validate_Int64Rules_msginit,
  &validate_UInt32Rules_msginit,
  &validate_UInt64Rules_msginit,
  &validate_SInt32Rules_msginit,
  &validate_SInt64Rules_msginit,
  &validate_Fixed32Rules_msginit,
  &validate_Fixed64Rules_msginit,
  &validate_SFixed32Rules_msginit,
  &validate_SFixed64Rules_msginit,
  &validate_BoolRules_msginit,
  &validate_StringRules_msginit,
  &validate_BytesRules_msginit,
  &validate_EnumRules_msginit,
  &validate_MessageRules_msginit,
  &validate_RepeatedRules_msginit,
  &validate_MapRules_msginit,
  &validate_AnyRules_msginit,
  &validate_DurationRules_msginit,
  &validate_TimestampRules_msginit,
};

static const char descriptor[5417] =
  "\n\027validate/validate.proto\022\010validate\032 goo"
  "gle/protobuf/descriptor.proto\032\036google/pr"
  "otobuf/duration.proto\032\037google/protobuf/t"
  "imestamp.proto\"\312\010\n\nFieldRules\022,\n\005float\030\001"
  " \001(\0132\024.validate.FloatRulesH\000R\005float\022/\n\006d"
  "ouble\030\002 \001(\0132\025.validate.DoubleRulesH\000R\006do"
  "uble\022,\n\005int32\030\003 \001(\0132\024.validate.Int32Rule"
  "sH\000R\005int32\022,\n\005int64\030\004 \001(\0132\024.validate.Int"
  "64RulesH\000R\005int64\022/\n\006uint32\030\005 \001(\0132\025.valid"
  "ate.UInt32RulesH\000R\006uint32\022/\n\006uint64\030\006 \001("
  "\0132\025.validate.UInt64RulesH\000R\006uint64\022/\n\006si"
  "nt32\030\007 \001(\0132\025.validate.SInt32RulesH\000R\006sin"
  "t32\022/\n\006sint64\030\010 \001(\0132\025.validate.SInt64Rul"
  "esH\000R\006sint64\0222\n\007fixed32\030\t \001(\0132\026.validate"
  ".Fixed32RulesH\000R\007fixed32\0222\n\007fixed64\030\n \001("
  "\0132\026.validate.Fixed64RulesH\000R\007fixed64\0225\n\010"
  "sfixed32\030\013 \001(\0132\027.validate.SFixed32RulesH"
  "\000R\010sfixed32\0225\n\010sfixed64\030\014 \001(\0132\027.validate"
  ".SFixed64RulesH\000R\010sfixed64\022)\n\004bool\030\r \001(\013"
  "2\023.validate.BoolRulesH\000R\004bool\022/\n\006string\030"
  "\016 \001(\0132\025.validate.StringRulesH\000R\006string\022,"
  "\n\005bytes\030\017 \001(\0132\024.validate.BytesRulesH\000R\005b"
  "ytes\022)\n\004enum\030\020 \001(\0132\023.validate.EnumRulesH"
  "\000R\004enum\0222\n\007message\030\021 \001(\0132\026.validate.Mess"
  "ageRulesH\000R\007message\0225\n\010repeated\030\022 \001(\0132\027."
  "validate.RepeatedRulesH\000R\010repeated\022&\n\003ma"
  "p\030\023 \001(\0132\022.validate.MapRulesH\000R\003map\022&\n\003an"
  "y\030\024 \001(\0132\022.validate.AnyRulesH\000R\003any\0225\n\010du"
  "ration\030\025 \001(\0132\027.validate.DurationRulesH\000R"
  "\010duration\0228\n\ttimestamp\030\026 \001(\0132\030.validate."
  "TimestampRulesH\000R\ttimestampB\006\n\004type\"\215\001\n\n"
  "FloatRules\022\024\n\005const\030\001 \001(\002R\005const\022\016\n\002lt\030\002"
  " \001(\002R\002lt\022\020\n\003lte\030\003 \001(\002R\003lte\022\016\n\002gt\030\004 \001(\002R\002"
  "gt\022\020\n\003gte\030\005 \001(\002R\003gte\022\016\n\002in\030\006 \003(\002R\002in\022\025\n\006"
  "not_in\030\007 \003(\002R\005notIn\"\216\001\n\013DoubleRules\022\024\n\005c"
  "onst\030\001 \001(\001R\005const\022\016\n\002lt\030\002 \001(\001R\002lt\022\020\n\003lte"
  "\030\003 \001(\001R\003lte\022\016\n\002gt\030\004 \001(\001R\002gt\022\020\n\003gte\030\005 \001(\001"
  "R\003gte\022\016\n\002in\030\006 \003(\001R\002in\022\025\n\006not_in\030\007 \003(\001R\005n"
  "otIn\"\215\001\n\nInt32Rules\022\024\n\005const\030\001 \001(\005R\005cons"
  "t\022\016\n\002lt\030\002 \001(\005R\002lt\022\020\n\003lte\030\003 \001(\005R\003lte\022\016\n\002g"
  "t\030\004 \001(\005R\002gt\022\020\n\003gte\030\005 \001(\005R\003gte\022\016\n\002in\030\006 \003("
  "\005R\002in\022\025\n\006not_in\030\007 \003(\005R\005notIn\"\215\001\n\nInt64Ru"
  "les\022\024\n\005const\030\001 \001(\003R\005const\022\016\n\002lt\030\002 \001(\003R\002l"
  "t\022\020\n\003lte\030\003 \001(\003R\003lte\022\016\n\002gt\030\004 \001(\003R\002gt\022\020\n\003g"
  "te\030\005 \001(\003R\003gte\022\016\n\002in\030\006 \003(\003R\002in\022\025\n\006not_in\030"
  "\007 \003(\003R\005notIn\"\216\001\n\013UInt32Rules\022\024\n\005const\030\001 "
  "\001(\rR\005const\022\016\n\002lt\030\002 \001(\rR\002lt\022\020\n\003lte\030\003 \001(\rR"
  "\003lte\022\016\n\002gt\030\004 \001(\rR\002gt\022\020\n\003gte\030\005 \001(\rR\003gte\022\016"
  "\n\002in\030\006 \003(\rR\002in\022\025\n\006not_in\030\007 \003(\rR\005notIn\"\216\001"
  "\n\013UInt64Rules\022\024\n\005const\030\001 \001(\004R\005const\022\016\n\002l"
  "t\030\002 \001(\004R\002lt\022\020\n\003lte\030\003 \001(\004R\003lte\022\016\n\002gt\030\004 \001("
  "\004R\002gt\022\020\n\003gte\030\005 \001(\004R\003gte\022\016\n\002in\030\006 \003(\004R\002in\022"
  "\025\n\006not_in\030\007 \003(\004R\005notIn\"\216\001\n\013SInt32Rules\022\024"
  "\n\005const\030\001 \001(\021R\005const\022\016\n\002lt\030\002 \001(\021R\002lt\022\020\n\003"
  "lte\030\003 \001(\021R\003lte\022\016\n\002gt\030\004 \001(\021R\002gt\022\020\n\003gte\030\005 "
  "\001(\021R\003gte\022\016\n\002in\030\006 \003(\021R\002in\022\025\n\006not_in\030\007 \003(\021"
  "R\005notIn\"\216\001\n\013SInt64Rules\022\024\n\005const\030\001 \001(\022R\005"
  "const\022\016\n\002lt\030\002 \001(\022R\002lt\022\020\n\003lte\030\003 \001(\022R\003lte\022"
  "\016\n\002gt\030\004 \001(\022R\002gt\022\020\n\003gte\030\005 \001(\022R\003gte\022\016\n\002in\030"
  "\006 \003(\022R\002in\022\025\n\006not_in\030\007 \003(\022R\005notIn\"\217\001\n\014Fix"
  "ed32Rules\022\024\n\005const\030\001 \001(\007R\005const\022\016\n\002lt\030\002 "
  "\001(\007R\002lt\022\020\n\003lte\030\003 \001(\007R\003lte\022\016\n\002gt\030\004 \001(\007R\002g"
  "t\022\020\n\003gte\030\005 \001(\007R\003gte\022\016\n\002in\030\006 \003(\007R\002in\022\025\n\006n"
  "ot_in\030\007 \003(\007R\005notIn\"\217\001\n\014Fixed64Rules\022\024\n\005c"
  "onst\030\001 \001(\006R\005const\022\016\n\002lt\030\002 \001(\006R\002lt\022\020\n\003lte"
  "\030\003 \001(\006R\003lte\022\016\n\002gt\030\004 \001(\006R\002gt\022\020\n\003gte\030\005 \001(\006"
  "R\003gte\022\016\n\002in\030\006 \003(\006R\002in\022\025\n\006not_in\030\007 \003(\006R\005n"
  "otIn\"\220\001\n\rSFixed32Rules\022\024\n\005const\030\001 \001(\017R\005c"
  "onst\022\016\n\002lt\030\002 \001(\017R\002lt\022\020\n\003lte\030\003 \001(\017R\003lte\022\016"
  "\n\002gt\030\004 \001(\017R\002gt\022\020\n\003gte\030\005 \001(\017R\003gte\022\016\n\002in\030\006"
  " \003(\017R\002in\022\025\n\006not_in\030\007 \003(\017R\005notIn\"\220\001\n\rSFix"
  "ed64Rules\022\024\n\005const\030\001 \001(\020R\005const\022\016\n\002lt\030\002 "
  "\001(\020R\002lt\022\020\n\003lte\030\003 \001(\020R\003lte\022\016\n\002gt\030\004 \001(\020R\002g"
  "t\022\020\n\003gte\030\005 \001(\020R\003gte\022\016\n\002in\030\006 \003(\020R\002in\022\025\n\006n"
  "ot_in\030\007 \003(\020R\005notIn\"!\n\tBoolRules\022\024\n\005const"
  "\030\001 \001(\010R\005const\"\374\003\n\013StringRules\022\024\n\005const\030\001"
  " \001(\tR\005const\022\020\n\003len\030\023 \001(\004R\003len\022\027\n\007min_len"
  "\030\002 \001(\004R\006minLen\022\027\n\007max_len\030\003 \001(\004R\006maxLen\022"
  "\033\n\tlen_bytes\030\024 \001(\004R\010lenBytes\022\033\n\tmin_byte"
  "s\030\004 \001(\004R\010minBytes\022\033\n\tmax_bytes\030\005 \001(\004R\010ma"
  "xBytes\022\030\n\007pattern\030\006 \001(\tR\007pattern\022\026\n\006pref"
  "ix\030\007 \001(\tR\006prefix\022\026\n\006suffix\030\010 \001(\tR\006suffix"
  "\022\032\n\010contains\030\t \001(\tR\010contains\022\016\n\002in\030\n \003(\t"
  "R\002in\022\025\n\006not_in\030\013 \003(\tR\005notIn\022\026\n\005email\030\014 \001"
  "(\010H\000R\005email\022\034\n\010hostname\030\r \001(\010H\000R\010hostnam"
  "e\022\020\n\002ip\030\016 \001(\010H\000R\002ip\022\024\n\004ipv4\030\017 \001(\010H\000R\004ipv"
  "4\022\024\n\004ipv6\030\020 \001(\010H\000R\004ipv6\022\022\n\003uri\030\021 \001(\010H\000R\003"
  "uri\022\031\n\007uri_ref\030\022 \001(\010H\000R\006uriRefB\014\n\nwell_k"
  "nown\"\277\002\n\nBytesRules\022\024\n\005const\030\001 \001(\014R\005cons"
  "t\022\020\n\003len\030\r \001(\004R\003len\022\027\n\007min_len\030\002 \001(\004R\006mi"
  "nLen\022\027\n\007max_len\030\003 \001(\004R\006maxLen\022\030\n\007pattern"
  "\030\004 \001(\tR\007pattern\022\026\n\006prefix\030\005 \001(\014R\006prefix\022"
  "\026\n\006suffix\030\006 \001(\014R\006suffix\022\032\n\010contains\030\007 \001("
  "\014R\010contains\022\016\n\002in\030\010 \003(\014R\002in\022\025\n\006not_in\030\t "
  "\003(\014R\005notIn\022\020\n\002ip\030\n \001(\010H\000R\002ip\022\024\n\004ipv4\030\013 \001"
  "(\010H\000R\004ipv4\022\024\n\004ipv6\030\014 \001(\010H\000R\004ipv6B\014\n\nwell"
  "_known\"k\n\tEnumRules\022\024\n\005const\030\001 \001(\005R\005cons"
  "t\022!\n\014defined_only\030\002 \001(\010R\013definedOnly\022\016\n\002"
  "in\030\003 \003(\005R\002in\022\025\n\006not_in\030\004 \003(\005R\005notIn\">\n\014M"
  "essageRules\022\022\n\004skip\030\001 \001(\010R\004skip\022\032\n\010requi"
  "red\030\002 \001(\010R\010required\"\215\001\n\rRepeatedRules\022\033\n"
  "\tmin_items\030\001 \001(\004R\010minItems\022\033\n\tmax_items\030"
  "\002 \001(\004R\010maxItems\022\026\n\006unique\030\003 \001(\010R\006unique\022"
  "*\n\005items\030\004 \001(\0132\024.validate.FieldRulesR\005it"
  "ems\"\271\001\n\010MapRules\022\033\n\tmin_pairs\030\001 \001(\004R\010min"
  "Pairs\022\033\n\tmax_pairs\030\002 \001(\004R\010maxPairs\022\033\n\tno"
  "_sparse\030\003 \001(\010R\010noSparse\022(\n\004keys\030\004 \001(\0132\024."
  "validate.FieldRulesR\004keys\022,\n\006values\030\005 \001("
  "\0132\024.validate.FieldRulesR\006values\"M\n\010AnyRu"
  "les\022\032\n\010required\030\001 \001(\010R\010required\022\016\n\002in\030\002 "
  "\003(\tR\002in\022\025\n\006not_in\030\003 \003(\tR\005notIn\"\351\002\n\rDurat"
  "ionRules\022\032\n\010required\030\001 \001(\010R\010required\022/\n\005"
  "const\030\002 \001(\0132\031.google.protobuf.DurationR\005"
  "const\022)\n\002lt\030\003 \001(\0132\031.google.protobuf.Dura"
  "tionR\002lt\022+\n\003lte\030\004 \001(\0132\031.google.protobuf."
  "DurationR\003lte\022)\n\002gt\030\005 \001(\0132\031.google.proto"
  "buf.DurationR\002gt\022+\n\003gte\030\006 \001(\0132\031.google.p"
  "rotobuf.DurationR\003gte\022)\n\002in\030\007 \003(\0132\031.goog"
  "le.protobuf.DurationR\002in\0220\n\006not_in\030\010 \003(\013"
  "2\031.google.protobuf.DurationR\005notIn\"\363\002\n\016T"
  "imestampRules\022\032\n\010required\030\001 \001(\010R\010require"
  "d\0220\n\005const\030\002 \001(\0132\032.google.protobuf.Times"
  "tampR\005const\022*\n\002lt\030\003 \001(\0132\032.google.protobu"
  "f.TimestampR\002lt\022,\n\003lte\030\004 \001(\0132\032.google.pr"
  "otobuf.TimestampR\003lte\022*\n\002gt\030\005 \001(\0132\032.goog"
  "le.protobuf.TimestampR\002gt\022,\n\003gte\030\006 \001(\0132\032"
  ".google.protobuf.TimestampR\003gte\022\025\n\006lt_no"
  "w\030\007 \001(\010R\005ltNow\022\025\n\006gt_now\030\010 \001(\010R\005gtNow\0221\n"
  "\006within\030\t \001(\0132\031.google.protobuf.Duration"
  "R\006within:=\n\010disabled\022\037.google.protobuf.M"
  "essageOptions\030\227\2158 \001(\010R\010disabled:;\n\010requi"
  "red\022\035.google.protobuf.OneofOptions\030\227\2158 \001"
  "(\010R\010required:K\n\005rules\022\035.google.protobuf."
  "FieldOptions\030\227\2158 \001(\0132\024.validate.FieldRul"
  "esR\005rulesB.Z,github.com/lyft/protoc-gen-"
  "validate/validate";

static upb_def_init *deps[4] = {
  &google_protobuf_descriptor_proto_upbdefinit,
  &google_protobuf_duration_proto_upbdefinit,
  &google_protobuf_timestamp_proto_upbdefinit,
  NULL
};

upb_def_init validate_validate_proto_upbdefinit = {
  deps,
  layouts,
  "validate/validate.proto",
  UPB_STRVIEW_INIT(descriptor, 5417)
};
