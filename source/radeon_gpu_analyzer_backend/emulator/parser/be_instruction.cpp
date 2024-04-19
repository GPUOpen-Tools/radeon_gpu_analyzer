//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <algorithm>
#include <sstream>

// Local.
#include "source/radeon_gpu_analyzer_backend/emulator/parser/be_instruction.h"
#include "source/common/rga_cli_defs.h"

// *******************
// ISA OPCODES - BEGIN
// *******************

// ***************************
// SCALAR INSTRUCTIONS - BEGIN
// ***************************
static const char* kSop2SAddU32 = "s_add_u32";
static const char* kSop2SSubU32 = "s_sub_u32";
static const char* kSop2SAddI32 = "s_add_i32";
static const char* kSop2SSubI32 = "s_sub_i32";
static const char* kSop2SAddcU32 = "s_addc_u32";
static const char* kSop2SSubbU32 = "s_subb_u32";
static const char* kSop2SLshlB32 = "s_lshl_b32";
static const char* kSop2SLshlB64 = "s_lshl_b64";
static const char* kSop2SLshrB32 = "s_lshr_b32";
static const char* kSop2SLshrB64 = "s_lshr_b64";
static const char* kSop2SMulI32 = "s_mul_i32";
static const char* kSop2SAndB32 = "s_and_b32";
static const char* kSop2SAndB64 = "s_and_b64";
static const char* kSop2SOrB32 = "s_or_b32";
static const char* kSop2SOrB64 = "s_or_b64";
static const char* kSop2SXorB32 = "s_xor_b32";
static const char* kSop2SXorB64 = "s_xor_b64";
static const char* kSop2SAndn2B32 = "s_andn2_b32";
static const char* kSop2SAndn2B64 = "s_andn2_b64";
static const char* kSop2SOrn2B32 = "s_orn2_b32";
static const char* kSop2SOrn2B64 = "s_orn2_b64";
static const char* kSop2SNandB32 = "s_nand_b32";
static const char* kSop2SNandB64 = "s_nand_b64";
static const char* kSop2SNorB32 = "s_nor_b32";
static const char* kSop2SNorB64 = "s_nor_b64";
static const char* kSop2SXnorB32 = "s_xnor_b32";
static const char* kSop2SXnorB64 = "s_xnor_b64";
static const char* kSop1SMovB32 = "s_mov_b32";
static const char* kSop1SMovB64 = "s_mov_b64";
static const char* kSop1SCmovB32 = "s_cmov_b32";
static const char* kSop1SCmovB64 = "s_cmov_b64";
static const char* kSop1SNotB32 = "s_not_b32";
static const char* kSop1SNotB64 = "s_not_b64";

// Comparison.
static const char* kSopcSCmpEqI32 = "s_cmp_eq_i32";
static const char* kSopcSCmpNeI32 = "s_cmp_ne_i32";
static const char* kSopcSCmpGtI32 = "s_cmp_gt_i32";
static const char* kSopcSCmpGeI32 = "s_cmp_ge_i32";
static const char* kSopcSCmpLeI32 = "s_cmp_le_i32";
static const char* kSopcSCmpLtI32 = "s_cmp_lt_i32";

static const char* kSopcSCmpEqU32 = "s_cmp_eq_u32";
static const char* kSopcSCmpNeU32 = "s_cmp_ne_u32";
static const char* kSopcSCmpGtU32 = "s_cmp_gt_u32";
static const char* kSopcSCmpGeU32 = "s_cmp_ge_u32";
static const char* kSopcSCmpLeU32 = "s_cmp_le_u32";
static const char* kSopcSCmpLtU32 = "s_cmp_lt_u32";

static const char* kSopkSCmpkEqI32 = "s_cmpk_eq_i32";
static const char* kSopkSCmpkNeI32 = "s_cmpk_ne_i32";
static const char* kSopkSCmpkGtI32 = "s_cmpk_gt_i32";
static const char* kSopkSCmpkGeI32 = "s_cmpk_ge_i32";
static const char* kSopkSCmpkLeI32 = "s_cmpk_le_i32";
static const char* kSopkSCmpkLtI32 = "s_cmpk_lt_i32";

static const char* kSopkSCmpkEqU32 = "s_cmpk_eq_u32";
static const char* kSopkSCmpkNeU32 = "s_cmpk_ne_u32";
static const char* kSopkSCmpkGtU32 = "s_cmpk_gt_u32";
static const char* kSopkSCmpkGeU32 = "s_cmpk_ge_u32";
static const char* kSopkSCmpkLeU32 = "s_cmpk_le_u32";
static const char* kSopkSCmpkLtU32 = "s_cmpk_lt_u32";

static const char* kSopcSBitcmp0B32 = "s_bitcmp0_b32";
static const char* kSopcSBitcmp0B64 = "s_bitcmp0_b64";

static const char* kSopcSBItcmp1B32 = "s_bitcmp1_b32";
static const char* kSopcSBItcmp1B64 = "s_bitcmp1_b64";

// Other.
static const char* kSop2SMovkI32 = "s_movk_i32";
static const char* kSop2SAshrI32 = "s_ashr_i32";
static const char* kSop2SAshrI64 = "s_ashr_i64";
static const char* kSop2SBfmB32 = "s_bfm_b32";
static const char* kSop2SBfmB64 = "s_bfm_b64";

static const char* kSop2SBfeI32 = "s_bfe_i32";
static const char* kSop2SBfeI64 = "s_bfe_i64";
static const char* kSop2SBfeU32 = "s_bfe_u32";
static const char* kSop2SBfeU64 = "s_bfe_u64";

static const char* kSop1SWqmB32 = "s_wqm_b32";
static const char* kSop1SWqmB64 = "s_wqm_b64";

static const char* kSop1SQuadmaskB32 = "s_quadmask_b32";
static const char* kSop1SQuadmaskB64 = "s_quadmask_b64";

static const char* kSop1SBrevB32 = "s_brev_b32";
static const char* kSop1SBrevB64 = "s_brev_b64";

static const char* kSop1SBCnt0I32B32 = "s_bcnt0_i32_b32";
static const char* kSop1SBCnt0I32B64 = "s_bcnt0_i32_b64";

static const char* kSop1SBcnt1I32B32 = "s_bcnt1_i32_b32";
static const char* kSop1SBcnt1I32B64 = "s_bcnt1_i32_b64";

static const char* kSop1SFf0I32B32 = "s_ff0_i32_b32";
static const char* kSop1SFf0I32B64 = "s_ff0_i32_b64";
static const char* kSop1SFf1I32B32 = "s_ff1_i32_b32";
static const char* kSop1SFf1I32B64 = "s_ff1_i32_b64";

static const char* kSop1SFlbitI32B32 = "s_flbit_i32_b32";
static const char* kSop1SFlbitI32B64 = "s_flbit_i32_b64";

static const char* kSop1SFlbitI32 = "s_flbit_i32";
static const char* kSop1SFlbitI32I64 = "s_flbit_i32_i64";

static const char* kSop1SBitset0B32 = "s_bitset0_b32";
static const char* kSop1SBitset0B64 = "s_bitset0_b64";
static const char* kSop1SBitset1B32 = "s_bitset1_b32";
static const char* kSop1SBitset1B64 = "s_bitset1_b64";

static const char* kSop1SAndSaveexecB64 = "s_and_saveexec_b64";
static const char* kSop1SOrSaveexecB64 = "s_or_saveexec_b64";
static const char* kSop1SXorSaveexecB64 = "s_xor_saveexec_b64";
static const char* kSop1SAndn2SaveexecB64 = "s_andn2_saveexec_b64";
static const char* kSop1SOrn2SaveexecB64 = "s_orn2_saveexec_b64";
static const char* kSop1SNandSaveexecB64 = "s_nand_saveexec_b64";
static const char* kSop1SNorSaveexecB64 = "s_nor_saveexec_b64";
static const char* kSop1SXnorSaveexecB64 = "s_xnor_saveexec_b64";

static const char* kSop1SMovrelsB32 = "s_movrels_b32";
static const char* kSop1SMovrelsB64 = "s_movrels_b64";
static const char* kSop1SMovreldB32 = "s_movreld_b32";
static const char* kSop1SmovreldB64 = "s_movreld_b64";

static const char* kSopkSGetregB32 = "s_getreg_b32";
static const char* kSopkSSetregB32 = "s_setreg_b32";
static const char* kSopkSSetregimm32B32 = "s_setreg_imm32_b32";

static const char* kSop2SCselectB32 = "s_cselect_b32";
static const char* kSop2SCselectB64 = "s_cselect_b64";

static const char* kSopkSCMovkI32 = "s_cmovk_i32";

static const char* kSop2SAbsdiffI32 = "s_absdiff_i32";
static const char* kSop2SMinI32 = "s_min_i32";
static const char* kSop2SMinU32 = "s_min_u32";
static const char* kSop2SMaxI32 = "s_max_i32";
static const char* kSop2SMaxU32 = "s_max_u32";
static const char* kSop1SAbsI32 = "s_abs_i32";
static const char* kSop1SSextI32I8 = "s_sext_i32_i8";
static const char* kSop1SSextI32I16 = "s_sext_i32_i16";

// SOPP - Program Control.
static const char* kSoppSNop = "s_nop";
static const char* kSoppSEndpgm = "s_endpgm";
static const char* kSoppSTrap = "s_trap";
static const char* kSoppSRfe = "s_rfe";
static const char* kSoppSSetprio = "s_setprio";
static const char* kSoppSSleep = "s_sleep";
static const char* kSoppSSendmsg = "s_sendmsg";
static const char* kSoppSBarrier = "s_barrier";
static const char* kSoppSSethalt = "s_sethalt";
static const char* kSoppSSendmsghalt = "s_sendmsghalt";
static const char* kSoppSIcache_inv = "s_icache_inv";
static const char* kSoppSIncperflevel = "s_incperflevel";
static const char* kSoppSDecperflevel = "s_decperflevel";
static const char* kSoppSTtracedata = "s_ttracedata";

// SOPP - Branch.
static const char* kSoppSBranch = "s_branch";
static const char* kSoppSCbranchScc0 = "s_cbranch_scc0";
static const char* kSoppSCbranchScc1 = "s_cbranch_scc1";
static const char* kSoppSCbranchVccz = "s_cbranch_vccz";
static const char* kSoppSCbranchVccnz = "s_cbranch_vccnz";
static const char* kSoppSCbranchExecz = "s_cbranch_execz";
static const char* kSoppSCbranchExecnz = "s_cbranch_execnz";
static const char* kSoppSSetpc = "s_setpc";
static const char* kSoppSSwappc = "s_swappc";

// Prefixes.
static const char* kSoppSCondBranchPrefix = "s_cbranch_";

// Ops.
static const char* kOpSWaitcnt = "s_waitcnt";

// *************************
// SCALAR INSTRUCTIONS - END
// *************************


// ***************************
// VECTOR INSTRUCTIONS - BEGIN
// ***************************

// FULL-RATE
// Not implemented by the hardware (handled by sequencer).
static const char* kVop2NoHwImplVMacF32 = "v_mac_f32";
static const char* kVop2NoHwImplVMacLegacyF32 = "v_mac_legacy_f32";
static const char* kVop2NoHwImplVSubbrevU32 = "v_subbrev_u32";
static const char* kVop2NoHwImplVSubF32 = "v_sub_f32";
static const char* kVop2NoHwImplVSubrevF32 = "v_subrev_f32";
static const char* kVop2NoHwImplVSubrevI32 = "v_subrev_i32";
static const char* kVop2NoHwImplVMadmkF32 = "v_madmk_f32";
static const char* kVop2NoHwImplVMadakF32 = "v_madak_f32";
static const char* kVop2NoHwImplVSubbU32 = "v_subb_u32";
static const char* kVop2NoHwImplVLshrrevB32 = "v_lshrrev_b32";
static const char* kVop2NoHwImplVAshrrevI32 = "v_ashrrev_i32";
static const char* kVop2NoHwImplVLshlrevB32 = "v_lshlrev_b32";

// Not documented.
static const char* kVop2VMovFedB32 = "v_mov_fed_b32";
static const char* kVop2NodocVAddU32 = "v_add_u32";
static const char* kVop2NodocVSubU32 = "v_sub_u32";
static const char* kVop1VFractF32 = "v_fract_f32";
static const char* kVop1VTruncF32 = "v_trunc_f32";
static const char* kVop2VMaxLegacyF32 = "v_max_legacy_f32";
static const char* kVop2VMinLegacyF32 = "v_min_legacy_f32";
static const char* kVop2VMinF32 = "v_min_f32";
static const char* kVop2VMaxF32 = "v_max_f32";
static const char* kVop2VCeilF32 = "v_ceil_f32";
static const char* kVop2VRndneF32 = "v_rndne_f32";
static const char* kVop2VFloorF32 = "v_floor_f32";
static const char* kVop2VMinI32 = "v_min_i32";
static const char* kVop2VMaxU32 = "v_max_u32";
static const char* kVop2VMinU32 = "v_min_u32";
static const char* kVop2VASHRI32 = "v_ashr_i32";
static const char* kVop1VMovB32 = "v_mov_b32";
static const char* kVop1VNotB32 = "v_not_b32";
static const char* kVop1VCvtF32I32 = "v_cvt_f32_i32";
static const char* kVop1VCvtF32U32 = "v_cvt_f32_u32";
static const char* kVop1VCvtU32F32 = "v_cvt_u32_f32";
static const char* kVop1VCvtI32F32 = "v_cvt_i32_f32";
static const char* kVop1VCvtRpiI32F32 = "v_cvt_rpi_i32_f32";
static const char* kVop1VCvtFlrI32F32 = "v_cvt_flr_i32_f32";
static const char* kVop1VCvtOffF32I4 = "v_cvt_off_f32_i4";
static const char* kVop1VBfrevB32 = "v_bfrev_b32";
static const char* kVop2VMulI32I24 = "v_mul_i32_i24";
static const char* kVop2VMulHII32I24 = "v_mul_hi_i32_i24";
static const char* kVop2VMulU32U24 = "v_mul_u32_u24";
static const char* kVop2VMulHIU32U24 = "v_mul_hi_u32_u24";
static const char* kVop2VMulF32 = "v_mul_f32";
static const char* kVop2VMulLegacyF32 = "v_mul_legacy_f32";
static const char* kVop2VAddF32 = "v_add_f32";
static const char* kVop2VAddI32 = "v_add_i32";
static const char* kVop2VAddcU32 = "v_addc_u32";
static const char* kVop2VSubI32 = "v_sub_i32";
static const char* kVop2VLshLB32 = "v_lshl_b32";
static const char* kVop2VXorB32 = "v_xor_b32";
static const char* kVop2VOrB32 = "v_or_b32";
static const char* kVop2VAndB32 = "v_and_b32";
static const char* kVop3VMadLegacyF32 = "v_mad_legacy_f32";
static const char* kVop3VMulLitF32 = "v_mullit_f32";
static const char* kVop3VAddF64 = "v_add_f64";
static const char* kVop3VMadI32I24 = "v_mad_i32_i24";
static const char* kVop3VMadU32U24 = "v_mad_u32_u24";
static const char* kVop3VMadF32 = "v_mad_f32";
static const char* kVop2VBfeI32 = "v_bfe_i32";
static const char* kVop3VAlignbitB32 = "v_alignbit_b32";
static const char* kVop3VAlignbyteB32 = "v_alignbyte_b32";
static const char* kVop2VBfmB32 = "v_bfm_b32";
static const char* kVop3VBfiB32 = "v_bfi_b32";
static const char* kVop2VBcntU32B32 = "v_bcnt_u32_b32";
static const char* kVop2VMbcntLoU32B32 = "v_mbcnt_lo_u32_b32";
static const char* kVop2VMbcntHiU32B32 = "v_mbcnt_hi_u32_b32";
static const char* kVop1VFfbhU32 = "V_FFBH_U32";
static const char* kVop1VFfblB32 = "V_FFBL_B32";
static const char* kVop1VFfbhI32 = "V_FFBH_I32";
static const char* kVop3VSadU8 = "v_sad_u8";
static const char* kVop3VMSadU8 = "v_msad_u8";
static const char* kVop3VSADHiU8 = "v_sad_hi_u8";
static const char* kVop3VSADU16 = "v_sad_u16";
static const char* kVop3VSadU32 = "v_sad_u32";
static const char* kVop3VLerpU8 = "v_lerp_u8";
static const char* kVop1VCvtF32Ubyte0 = "v_cvt_f32_ubyte0";
static const char* kVop1VCvtF32Ubyte1 = "v_cvt_f32_ubyte1";
static const char* kVop1VCvtF32Ubyte2 = "v_cvt_f32_ubyte2";
static const char* kVop1VCvtF32Ubyte3 = "v_cvt_f32_ubyte3";
static const char* kVop1VCvtF16F32 = "v_cvt_f16_f32";
static const char* kVop1VCvtF32F16 = "v_cvt_f32_f16";
static const char* kVop2VCvtPkrtzF16F32 = "v_cvt_pkrtz_f16_f32";
static const char* kVop2VCvtPknormI16F32 = "v_cvt_pknorm_i16_f32";
static const char* kVop2VCvtPknormU16F32 = "v_cvt_pknorm_u16_f32";
static const char* kVop2VCvtPkU16U32 = "v_cvt_pk_u16_u32";
static const char* kVop2VCvtPkI16I32 = "v_cvt_pk_i16_i32";
static const char* kVop3VFmaF32 = "v_fma_f32";
static const char* kVop3VMulI32I24 = "v_mul_i32_i24";
static const char* kVop2VCndmaskB32 = "v_cndmask_b32";
static const char* kVop3VCubeidF32 = "v_cubeid_f32";
static const char* kVop3VCubescF32 = "v_cubesc_f32";
static const char* kVop3VCubetcF32 = "v_cubetc_f32";
static const char* kVop3VCubemaF32 = "v_cubema_f32";
static const char* kVop3VMaxF32 = "v_max3_f32";
static const char* kVop3VMaxI32 = "v_max3_i32";
static const char* kVop3VMaxU32 = "v_max3_u32";
static const char* kVop3VMin3F32 = "v_min3_f32";
static const char* kVop3VMin3I32 = "v_min3_i32";
static const char* kVop3VMin3U32 = "v_min3_u32";
static const char* kVop3VMed3F32 = "v_med3_f32";
static const char* kVop3VMed3I32 = "v_med3_i32";
static const char* kVop3VMed3U32 = "v_med3_u32";
static const char* kVop3VCvtpkU8F32 = "v_cvt_pk_u8_f32";
static const char* kVop1VFRexpMantF32 = "v_frexp_mant_f32";
static const char* kVop1VFRexpExpI32F32 = "v_frexp_exp_i32_f32";
static const char* kVop2VLDexpF32 = "v_ldexp_f32";
static const char* kVopcVCmpeqF32 = "v_cmp_eq_f32";
static const char* kVopcVCmpleF32 = "v_cmp_le_f32";
static const char* kVopcVCmpgtF32 = "v_cmp_gt_f32";
static const char* kVopcVCmplgF32 = "v_cmp_lg_f32";
static const char* kVopcVCmpgeF32 = "v_cmp_ge_f32";
static const char* kVopcVCmpOF32 = "v_cmp_o_f32";
static const char* kVopcVCmpUF32 = "v_cmp_u_f32";
static const char* kVopcVCmpNgeF32 = "v_cmp_nge_f32";
static const char* kVopcVCmpNlgF32 = "v_cmp_nlg_f32";
static const char* kVopcVCmpNgtF32 = "v_cmp_ngt_f32";
static const char* kVopcVCmpNleF32 = "v_cmp_nle_f32";
static const char* kVopcVCmpNeqF32 = "v_cmp_neq_f32";
static const char* kVopcVCmpNltF32 = "v_cmp_nlt_f32";
static const char* kVopcVCmpTruF32 = "v_cmp_tru_f32";
static const char* kVopcVCmpxNgeF32 = "v_cmpx_nge_f32";
static const char* kVopcVCmpxNlgF32 = "v_cmpx_nlg_f32";
static const char* kVopcVCmpxNgtF32 = "v_cmpx_ngt_f32";
static const char* kVopcVCmpxNleF32 = "v_cmpx_nle_f32";
static const char* kVopcVCmpxNeqF32 = "v_cmpx_neq_f32";
static const char* kVopcVCmpxNltF32 = "v_cmpx_nlt_f32";
static const char* kVopcVCmpxTruF32 = "v_cmpx_tru_f32";
static const char* kVopcVCmpFI32 = "v_cmp_f_i32";
static const char* kVopcVCmpLtI32 = "v_cmp_lt_i32";
static const char* kVopcVCmpEqI32 = "v_cmp_eq_i32";
static const char* kVopcVCmpLeI32 = "v_cmp_le_i32";
static const char* kVopcVCmpGtI32 = "v_cmp_gt_i32";
static const char* kVopcVCmpNeI32 = "v_cmp_ne_i32";
static const char* kVopcVCmpGeI32 = "v_cmp_ge_i32";
static const char* kVopcVCmpTI32 = "v_cmp_t_i32";
static const char* kVopcVCmpxFI32 = "v_cmpx_f_i32";
static const char* kVopcVCmpxLtI32 = "v_cmpx_lt_i32";
static const char* kVopcVCmpxEqI32 = "v_cmpx_eq_i32";
static const char* kVopcVCmpxLeI32 = "v_cmpx_le_i32";
static const char* kVopcVCmpxGtI32 = "v_cmpx_gt_i32";
static const char* kVopcVCmpxNeI32 = "v_cmpx_ne_i32";
static const char* kVopcVCmpxGeI32 = "v_cmpx_ge_i32";
static const char* kVopcVCmpxTI32 = "v_cmpx_t_i32";
static const char* kVopcVCmpFU32 = "v_cmp_f_u32";
static const char* kVopcVCmpLtU32 = "v_cmp_lt_u32";
static const char* kVopcVCmpEqU32 = "v_cmp_eq_u32";
static const char* kVopcVCmpLeU32 = "v_cmp_le_u32";
static const char* kVopcVCmpGtU32 = "v_cmp_gt_u32";
static const char* kVopcVCmpNeU32 = "v_cmp_ne_u32";
static const char* kVopcVCmpGeU32 = "v_cmp_ge_u32";
static const char* kVopcVCmpTU32 = "v_cmp_t_u32";
static const char* kVopcVCmpxFU32 = "v_cmpx_f_u32";
static const char* kVopcVCmpxLtU32 = "v_cmpx_lt_u32";
static const char* kVopcVCmpxEqU32 = "v_cmpx_eq_u32";
static const char* kVopcVCmpxLeU32 = "v_cmpx_le_u32";
static const char* kVopcVCmpxGtU32 = "v_cmpx_gt_u32";
static const char* kVopcVCmpxNeU32 = "v_cmpx_ne_u32";
static const char* kVopcVCmpxGeU32 = "v_cmpx_ge_u32";
static const char* kVopcVCmpxTU32 = "v_cmpx_t_u32";
static const char* kVopcVCmpClassF32 = "v_cmp_class_f32";

// New to Volcanic Islands (all Full Rate).
static const char* kVIVAddF16 = "v_add_f16";
static const char* kVIVMulF16 = "v_mul_f16";
static const char* kVIVMadF16 = "v_mad_f16";
static const char* kVIVFmaF16 = "v_fma_f16";
static const char* kVIVDivScaleF16 = "v_div_scale_f16";
static const char* kVIVDivFmasF16 = "v_div_fmas_f16";
static const char* kFra = "v_div_fixup_f16";
static const char* kViVFractF16 = "v_fract_f16";
static const char* kViVTruncF16 = "v_trunc_f16";
static const char* kViVCeilF16 = "v_ceil_f16";
static const char* kViVRndneF16 = "v_rndne_f16";
static const char* kViVFloorF16 = "v_floor_f16";
static const char* kViVFrexpMantF16 = "v_frexp_mant_f16";
static const char* kViVFrexpExpI16F16 = "v_frexp_exp_i16_f16";
static const char* kViVLdexpF16 = "v_ldexp_f16";
static const char* kViVMaxF16 = "v_max_f16";
static const char* kViVMinF16 = "v_min_f16";
static const char* kViVinterpp1llF16 = "v_interp_p1ll_f16";
static const char* kViVinterpp1lvF16 = "v_interp_p1lv_f16";
static const char* kViVinterpp2F16 = "v_interp_p2_f16";
static const char* kViVAddU16 = "v_add_u16";
static const char* kViVSubI16 = "v_sub_i16";
static const char* kViVSubU16 = "v_sub_u16";
static const char* kViVMulI16 = "v_mul_i16";
static const char* kViVMulU16 = "v_mul_u16";
static const char* kViVMadI16 = "v_mad_i16";
static const char* kViVMadU16 = "v_mad_u16";
static const char* kViVMaxI16 = "v_max_i16";
static const char* kViVMinI16 = "v_min_i16";
static const char* kViVMaxU16 = "v_max_u16";
static const char* kViVMinU16 = "v_min_u16";
static const char* kViVAshrI16 = "v_ashr_i16";
static const char* kViVLshrB16 = "v_lshr_b16";
static const char* kViVLshlB16 = "v_lshl_b16";
static const char* kViVCvtI16F16 = "v_cvt_i16_f16";
static const char* kViVCvtU16F16 = "v_cvt_u16_f16";
static const char* kViVCvtF16I16 = "v_cvt_f16_i16";
static const char* kViVCvtF16U16 = "v_cvt_f16_u16";
static const char* kViVCvtPERMB32 = "v_perm_b32";

// Full rate, 1/4 on hybrid architecture.
static const char* kVop3VDivScaleF32 = "v_div_scale_f32";
static const char* kVop3VDivFmasF32 = "v_div_fmas_f32";
static const char* kVop3VDivFixupF32 = "v_div_fixup_f32";

// HALF RATE.
static const char* kVop1VLogF32 = "v_log_f32";

// 1/2 rate, 1/8 on hybrid architecture:
static const char* kVop1VCvtF32F64 = "v_cvt_f32_f64";
static const char* kVop1VCvtI32F64 = "v_cvt_i32_f64";
static const char* kVop1VCvtF64I32 = "v_cvt_f64_i32";
static const char* kVop1VCvtU32F64 = "v_cvt_u32_f64";
static const char* kVop1VCvtF64U32 = "v_cvt_f64_u32";
static const char* kVop3VMinF64 = "v_min_f64";
static const char* kVop3VMaxF64 = "v_max_f64";
static const char* kVop3VLdexPF64 = "v_ldexp_f64";
static const char* kVop3VFrexPMantF64 = "v_frexp_mant_f64";
static const char* kVop3VFrexPExpI32F64 = "v_frexp_exp_i32_f64";
static const char* kVop1VFracTF64 = "v_fract_f64";
static const char* kVop1VTrunCF64 = "v_trunc_f64";
static const char* kVop1VCeilF64 = "v_ceil_f64";
static const char* kVop1VRndnEF64 = "v_rndne_f64";
static const char* kVop1VFlooRF64 = "v_floor_f64";
static const char* kVop3VAshrI64 = "v_ashr_i64";
static const char* kVop3VDivScaleF64 = "v_div_scale_f64";
static const char* kVop3VDivFixupF64 = "v_div_fixup_f64";
static const char* kVopcVCmpFF64 = "v_cmp_f_f64";
static const char* kVopcVCmpLtF64 = "v_cmp_lt_f64";
static const char* kVopcVCmpEqF64 = "v_cmp_eq_f64";
static const char* kVopcVCmpLeF64 = "v_cmp_le_f64";
static const char* kVopcVCmpGtF64 = "v_cmp_gt_f64";
static const char* kVopcVCmpLgF64 = "v_cmp_lg_f64";
static const char* kVopcVCmpGeF64 = "v_cmp_ge_f64";
static const char* kVopcVCmpOF64 = "v_cmp_o_f64";
static const char* kVopcVCmpUF64 = "v_cmp_u_f64";
static const char* kVopcVCmpNgeF64 = "v_cmp_nge_f64";
static const char* kVopcVCmpNlgF64 = "v_cmp_nlg_f64";
static const char* kVopcVCmpNgtF64 = "v_cmp_ngt_f64";
static const char* kVopcVCmpNleF64 = "v_cmp_nle_f64";
static const char* kVopcVCmpNeqF64 = "v_cmp_neq_f64";
static const char* kVopcVCmpNltF64 = "v_cmp_nlt_f64";
static const char* kVopcVCmpTruF64 = "v_cmp_tru_f64";
static const char* kVopcVCmpxFF64 = "v_cmpx_f_f64";
static const char* kVopcVCmpxLtF64 = "v_cmpx_lt_f64";
static const char* kVopcVCmpxEqF64 = "v_cmpx_eq_f64";
static const char* kVopcVCmpxLeF64 = "v_cmpx_le_f64";
static const char* kVopcVCmpxGtF64 = "v_cmpx_gt_f64";
static const char* kVopcVCmpxLgF64 = "v_cmpx_lg_f64";
static const char* kVopcVCmpxGeF64 = "v_cmpx_ge_f64";
static const char* kVopcVCmpxOF64 = "v_cmpx_o_f64";
static const char* kVopcVCmpxUF64 = "v_cmpx_u_f64";
static const char* kVopcVCmpxNgeF64 = "v_cmpx_nge_f64";
static const char* kVopcVCmpxNlgF64 = "v_cmpx_nlg_f64";
static const char* kVopcVCmpxNgtF64 = "v_cmpx_ngt_f64";
static const char* kVopcVCmpxNleF64 = "v_cmpx_nle_f64";
static const char* kVopcVCmpxNeqF64 = "v_cmpx_neq_f64";
static const char* kVopcVCmpxNltF64 = "v_cmpx_nlt_f64";
static const char* kVopcVCmpxTruF64 = "v_cmpx_tru_f64";
static const char* kVopcVCmpFI64 = "v_cmp_f_i64";
static const char* kVopcVCmpLtI64 = "v_cmp_lt_i64";
static const char* kVopcVCmpEqI64 = "v_cmp_eq_i64";
static const char* kVopcVCmpLeI64 = "v_cmp_le_i64";
static const char* kVopcVCmpGtI64 = "v_cmp_gt_i64";
static const char* kVopcVCmpLgI64 = "v_cmp_lg_i64";
static const char* kVopcVCmpGeI64 = "v_cmp_ge_i64";
static const char* kVopcVCmpTI64 = "v_cmp_t_i64";
static const char* kVopcVCmpxFI64 = "v_cmpx_f_i64";
static const char* kVopcVCmpxLtI64 = "v_cmpx_lt_i64";
static const char* kVopcVCmpxEqI64 = "v_cmpx_eq_i64";
static const char* kVopcVCmpxLeI64 = "v_cmpx_le_i64";
static const char* kVopcVCmpxGtI64 = "v_cmpx_gt_i64";
static const char* kVopcVCmpxLgI64 = "v_cmpx_lg_i64";
static const char* kVopcVCmpxGeI64 = "v_cmpx_ge_i64";
static const char* kVopcVCmpxTI64 = "v_cmpx_t_i64";
static const char* kVopcVCmpFU64 = "v_cmp_f_u64";
static const char* kVopcVCmpLtU64 = "v_cmp_lt_u64";
static const char* kVopcVCmpEqU64 = "v_cmp_eq_u64";
static const char* kVopcVCmpLeU64 = "v_cmp_le_u64";
static const char* kVopcVCmpGtU64 = "v_cmp_gt_u64";
static const char* kVopcVCmpLgU64 = "v_cmp_lg_u64";
static const char* kVopcVCmpGeU64 = "v_cmp_ge_u64";
static const char* kVopcVCmpTU64 = "v_cmp_t_u64";
static const char* kVopcVCmpxFU64 = "v_cmpx_f_u64";
static const char* kVopcVCmpxLtU64 = "v_cmpx_lt_u64";
static const char* kVopcVCmpxEqU64 = "v_cmpx_eq_u64";
static const char* kVopcVCmpxLeU64 = "v_cmpx_le_u64";
static const char* kVopcVCmpxGtU64 = "v_cmpx_gt_u64";
static const char* kVopcVCmpxLgU64 = "v_cmpx_lg_u64";
static const char* kVopcVCmpxGeU64 = "v_cmpx_ge_u64";
static const char* kVopcVCmpxTU64 = "v_cmpx_t_u64";
static const char* kVopcVCmpClassF64 = "v_cmp_class_f64";
static const char* kVopcVCmpxClassF64 = "v_cmpx_class_f64";

// QUARTER RATE
static const char* kVOP1VSqrtF32 = "v_sqrt_f32";
static const char* kVOP3VMulLoI32 = "v_mul_lo_i32";
static const char* kVOP3VMulHiI32 = "v_mul_hi_i32";
static const char* kVOP3VMulLoU32 = "v_mul_lo_u32";
static const char* kVOP3VMulHiU32 = "v_mul_hi_u32";
static const char* kVOP3VMadU64U32 = "v_mad_u64_u32";
static const char* kVOP3VMadI64I32 = "v_mad_i64_i32";
static const char* kVOP1VExpF32 = "v_exp_f32";
static const char* kVOP1VLogClampF32 = "v_log_clamp_f32";
static const char* kVOP1VRcpClampF32 = "v_rcp_clamp_f32";
static const char* kVOP1VRcpLegacyF32 = "v_rcp_legacy_f32";
static const char* kVOP1VRcpF32 = "v_rcp_f32";
static const char* kVOP1VRcpIFlagF32 = "v_rcp_iflag_f32";
static const char* kVOP1VRsqClampF32 = "v_rsq_clamp_f32";
static const char* kVOP1VRsqLegacyF32 = "v_rsq_legacy_f32";
static const char* kVOP1VRsqF32 = "v_rsq_f32";
static const char* kVOP1VSinF32 = "v_sin_f32";
static const char* kVOP1VCosF32 = "v_cos_f32";
static const char* kVOP1VRcpF64 = "v_rcp_f64";
static const char* kVOP1VRcpClampF64 = "v_rcp_clamp_f64";
static const char* kVOP1VRsqF64 = "v_rsq_f64";
static const char* kVOP1VRsqClampF64 = "v_rsq_clamp_f64";
static const char* kVOP3VTrigPreopF64 = "v_trig_preop_f64";
static const char* kVOP3VMulF64 = "v_mul_f64";
static const char* kVOP3VFmaF64 = "v_fma_f64";
static const char* kVOP3VDivFmasF64 = "v_div_fmas_f64";
static const char* kVOP3VMqsadPkU16U8 = "v_mqsad_pk_u16_u8";
static const char* kVOP3VMqsadU32U8 = "v_mqsad_u32_u8";

// QUARTER RATE - VI NEW OPCODES
static const char* kViVRcpF16 = "v_rcp_f16";
static const char* kViVSqrtF16 = "v_sqrt_f16";
static const char* kViVRsqF16 = "v_rsq_f16";
static const char* kViVExpF16 = "v_exp_f16";
static const char* kViVLogF16 = "v_log_f16";
static const char* kViVSinF16 = "v_sin_f16";
static const char* kViVCosF16 = "v_cos_f16";
static const char* kViQSadPkU16U8 = "v_qsad_pk_u16_u8";

// 1/4 rate, 1/16 on hybrid architecture.
static const char* kVop1VSqrtF64 = "v_sqrt_f64";

// Constants used for comma separated values string.
static const char* COMMA_SEPARATOR = ",";
static const char* NEWLINE_SEPARATOR = "\n";
static const char* DOUBLE_QUOTES = "\"";
static const char* NA_VALUE = "Varies";
static const char* BRANCH_CYCLES = "4|16";

// *************************
// VECTOR INSTRUCTIONS - END
// *************************

// Export instructions.
static const char* kExportExp = "EXP";

// *******************
// ISA OPCODES - END
// *******************


// Static members definition.
std::unordered_map<std::string, int> Instruction::half_device_perf_table_;
std::unordered_map<std::string, int> Instruction::quarter_device_perf_table_;
std::unordered_map<std::string, int> Instruction::hybrid_device_perf_table_;
std::unordered_map<std::string, int> Instruction::scalar_device_perf_table_;
bool Instruction::is_perf_tables_initialized_ = false;

int Instruction::GetInstructionClockCount(const std::string& device_name) const
{
    int ret = 0;

    if (!device_name.empty())
    {
        // Ignore the case.
        std::string opcode_lower_case(instruction_opcode_);
        std::transform(opcode_lower_case.begin(), opcode_lower_case.end(), opcode_lower_case.begin(), [](const char& c) { return static_cast<char>(std::tolower(c)); });

        // First look at the scalar performance table.
        auto dev_iter = scalar_device_perf_table_.find(opcode_lower_case);
        if (dev_iter != scalar_device_perf_table_.end())
        {
            ret = dev_iter->second;
        }
        else
        {
            dev_iter = hybrid_device_perf_table_.find(opcode_lower_case);
            if (dev_iter != hybrid_device_perf_table_.end())
            {
                ret = dev_iter->second;
            }
        }
    }

    return ret;
}

void Instruction::SetInstructionStringRepresentation(const std::string& opcode, const std::string& params, const std::string& binary_representation, const std::string& offset)
{
    instruction_opcode_ = opcode;
    std::transform(
        instruction_opcode_.begin(), instruction_opcode_.end(), instruction_opcode_.begin(), [](const char& c) { return static_cast<char>(std::tolower(c)); });

    parameters_ = params;
    binary_representation_ = binary_representation;
    offset_in_bytes_ = offset;

    // Deduce the instruction category.
    if ((instruction_opcode_.find(kSoppSCondBranchPrefix) != std::string::npos) ||
        instruction_opcode_ == kSoppSBranch || instruction_opcode_ == kSoppSSetpc ||
        instruction_opcode_ == kSoppSSwappc)
    {
        Instruction_category_ = InstructionCategory::kBranch;
    }
    else if (instruction_opcode_ == kSoppSEndpgm ||
             instruction_opcode_ == kOpSWaitcnt ||
             instruction_opcode_ == kSoppSNop ||
             instruction_opcode_ == kSoppSTrap ||
             instruction_opcode_ == kSoppSRfe ||
             instruction_opcode_ == kSoppSSetprio ||
             instruction_opcode_ == kSoppSSleep ||
             instruction_opcode_ == kSoppSSendmsg)
    {
        Instruction_category_ = InstructionCategory::kInternal;
    }
    else if (scalar_device_perf_table_.find(instruction_opcode_) != scalar_device_perf_table_.end())
    {
        Instruction_category_ = InstructionCategory::kScalarAlu;
    }
    else if (instruction_opcode_.compare(kExportExp) == 0)
    {
        Instruction_category_ = InstructionCategory::kExport;
    }
}

void Instruction::SetUpPerfTables()
{
    if (!is_perf_tables_initialized_)
    {
        // Scalar instructions.
        SetUpScalarPerfTables();

        // Hybrid architecture.
        SetUpHybridPerfTables();

        // Quarter double devices.
        SetUpQuarterDevicesPerfTables();

        // Half double devices.
        SetUpHalfDevicesPerfTables();

        // The tables have been initialized.
        is_perf_tables_initialized_ = true;
    }
}

void Instruction::SetUpHybridPerfTables()
{
    // Initialize the hybrid architecture device table.
    if (hybrid_device_perf_table_.empty())
    {
        // Full-rate.
        hybrid_device_perf_table_[kVop2NoHwImplVMacF32] = 4;
        hybrid_device_perf_table_[kVop2NoHwImplVMacLegacyF32] = 4;
        hybrid_device_perf_table_[kVop2NoHwImplVSubbrevU32] = 4;
        hybrid_device_perf_table_[kVop2NoHwImplVSubF32] = 4;
        hybrid_device_perf_table_[kVop2NoHwImplVSubrevF32] = 4;
        hybrid_device_perf_table_[kVop2NoHwImplVSubrevI32] = 4;
        hybrid_device_perf_table_[kVop2NoHwImplVMadmkF32] = 4;
        hybrid_device_perf_table_[kVop2NoHwImplVMadakF32] = 4;
        hybrid_device_perf_table_[kVop2NoHwImplVSubbU32] = 4;
        hybrid_device_perf_table_[kVop2NoHwImplVLshrrevB32] = 4;
        hybrid_device_perf_table_[kVop2NoHwImplVAshrrevI32] = 4;
        hybrid_device_perf_table_[kVop2NoHwImplVLshlrevB32] = 4;
        hybrid_device_perf_table_[kVop1VMovB32] = 4;
        hybrid_device_perf_table_[kVop2VMovFedB32] = 4;
        hybrid_device_perf_table_[kVop1VNotB32] = 4;
        hybrid_device_perf_table_[kVop1VCvtF32I32] = 4;
        hybrid_device_perf_table_[kVop1VCvtF32U32] = 4;
        hybrid_device_perf_table_[kVop1VCvtU32F32] = 4;
        hybrid_device_perf_table_[kVop1VCvtI32F32] = 4;
        hybrid_device_perf_table_[kVop1VCvtRpiI32F32] = 4;
        hybrid_device_perf_table_[kVop1VCvtFlrI32F32] = 4;
        hybrid_device_perf_table_[kVop1VCvtOffF32I4] = 4;
        hybrid_device_perf_table_[kVop1VBfrevB32] = 4;
        hybrid_device_perf_table_[kVop2VMulI32I24] = 4;
        hybrid_device_perf_table_[kVop2VMulHII32I24] = 4;
        hybrid_device_perf_table_[kVop2VMulU32U24] = 4;
        hybrid_device_perf_table_[kVop2VMulHIU32U24] = 4;
        hybrid_device_perf_table_[kVop2VMulF32] = 4;
        hybrid_device_perf_table_[kVop2VMulLegacyF32] = 4;
        hybrid_device_perf_table_[kVop2VAddF32] = 4;
        hybrid_device_perf_table_[kVop2VAddI32] = 4;
        hybrid_device_perf_table_[kVop2VAddcU32] = 4;
        hybrid_device_perf_table_[kVop2VSubI32] = 4;
        hybrid_device_perf_table_[kVop2NodocVAddU32] = 4;
        hybrid_device_perf_table_[kVop2NodocVSubU32] = 4;
        hybrid_device_perf_table_[kVop2VLshLB32] = 4;
        hybrid_device_perf_table_[kVop2VXorB32] = 4;
        hybrid_device_perf_table_[kVop2VOrB32] = 4;
        hybrid_device_perf_table_[kVop2VAndB32] = 4;
        hybrid_device_perf_table_[kVop3VMadLegacyF32] = 4;
        hybrid_device_perf_table_[kVop3VFmaF32] = 4;
        hybrid_device_perf_table_[kVop3VMulLitF32] = 4;
        hybrid_device_perf_table_[kVop3VAddF64] = 4;
        hybrid_device_perf_table_[kVop3VMadI32I24] = 4;
        hybrid_device_perf_table_[kVop3VMadU32U24] = 4;
        hybrid_device_perf_table_[kVop3VMadF32] = 4;
        hybrid_device_perf_table_[kVop2VBfeI32] = 4;
        hybrid_device_perf_table_[kVop3VAlignbitB32] = 4;
        hybrid_device_perf_table_[kVop3VAlignbyteB32] = 4;
        hybrid_device_perf_table_[kVop2VBfmB32] = 4;
        hybrid_device_perf_table_[kVop3VBfiB32] = 4;
        hybrid_device_perf_table_[kVop2VBcntU32B32] = 4;
        hybrid_device_perf_table_[kVop2VMbcntLoU32B32] = 4;
        hybrid_device_perf_table_[kVop2VMbcntHiU32B32] = 4;
        hybrid_device_perf_table_[kVop1VFfbhU32] = 4;
        hybrid_device_perf_table_[kVop1VFfblB32] = 4;
        hybrid_device_perf_table_[kVop1VFfbhI32] = 4;
        hybrid_device_perf_table_[kVop3VSadU8] = 4;
        hybrid_device_perf_table_[kVop3VMSadU8] = 4;
        hybrid_device_perf_table_[kVop3VSADHiU8] = 4;
        hybrid_device_perf_table_[kVop3VSADU16] = 4;
        hybrid_device_perf_table_[kVop3VSadU32] = 4;
        hybrid_device_perf_table_[kVop3VLerpU8] = 4;
        hybrid_device_perf_table_[kVop1VCvtF32Ubyte0] = 4;
        hybrid_device_perf_table_[kVop1VCvtF32Ubyte1] = 4;
        hybrid_device_perf_table_[kVop1VCvtF32Ubyte2] = 4;
        hybrid_device_perf_table_[kVop1VCvtF32Ubyte3] = 4;
        hybrid_device_perf_table_[kVop1VCvtOffF32I4] = 4;
        hybrid_device_perf_table_[kVop1VCvtF16F32] = 4;
        hybrid_device_perf_table_[kVop2VCvtPkrtzF16F32] = 4;
        hybrid_device_perf_table_[kVop1VCvtF32F16] = 4;
        hybrid_device_perf_table_[kVop2VCvtPknormI16F32] = 4;
        hybrid_device_perf_table_[kVop2VCvtPknormU16F32] = 4;
        hybrid_device_perf_table_[kVop2VCvtPkU16U32] = 4;
        hybrid_device_perf_table_[kVop2VCvtPkI16I32] = 4;
        hybrid_device_perf_table_[kVop3VFmaF32] = 4;
        hybrid_device_perf_table_[kVop2VMulU32U24] = 4;
        hybrid_device_perf_table_[kVop2VMulHIU32U24] = 4;
        hybrid_device_perf_table_[kVop3VMadU32U24] = 4;
        hybrid_device_perf_table_[kVop3VMulI32I24] = 4;
        hybrid_device_perf_table_[kVop2VMulHII32I24] = 4;
        hybrid_device_perf_table_[kVop3VMadI32I24] = 4;
        hybrid_device_perf_table_[kVop3VMulLitF32] = 4;
        hybrid_device_perf_table_[kVop2VCndmaskB32] = 4;
        hybrid_device_perf_table_[kVop3VCubeidF32] = 4;
        hybrid_device_perf_table_[kVop3VCubescF32] = 4;
        hybrid_device_perf_table_[kVop3VCubetcF32] = 4;
        hybrid_device_perf_table_[kVop3VCubemaF32] = 4;
        hybrid_device_perf_table_[kVop3VMaxF32] = 4;
        hybrid_device_perf_table_[kVop3VMaxI32] = 4;
        hybrid_device_perf_table_[kVop3VMaxU32] = 4;
        hybrid_device_perf_table_[kVop3VMin3F32] = 4;
        hybrid_device_perf_table_[kVop3VMin3I32] = 4;
        hybrid_device_perf_table_[kVop3VMin3U32] = 4;
        hybrid_device_perf_table_[kVop3VMed3F32] = 4;
        hybrid_device_perf_table_[kVop3VMed3I32] = 4;
        hybrid_device_perf_table_[kVop3VMed3U32] = 4;
        hybrid_device_perf_table_[kVop3VCvtpkU8F32] = 4;
        hybrid_device_perf_table_[kVop1VFRexpMantF32] = 4;
        hybrid_device_perf_table_[kVop1VFRexpExpI32F32] = 4;
        hybrid_device_perf_table_[kVop2VLDexpF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpeqF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpleF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpgtF32] = 4;
        hybrid_device_perf_table_[kVopcVCmplgF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpgeF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpOF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpUF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpNgeF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpNlgF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpNgtF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpNleF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpNeqF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpNltF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpTruF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxNgeF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxNlgF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxNgtF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxNleF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxNeqF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxNltF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxTruF32] = 4;
        hybrid_device_perf_table_[kVopcVCmpFI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpLtI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpEqI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpLeI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpGtI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpNeI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpGeI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpTI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxFI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxLtI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxEqI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxLeI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxGtI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxNeI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxGeI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxTI32] = 4;
        hybrid_device_perf_table_[kVopcVCmpFU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpLtU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpEqU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpLeU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpGtU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpNeU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpGeU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpTU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxFU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxLtU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxEqU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxLeU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxGtU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxNeU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxGeU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpxTU32] = 4;
        hybrid_device_perf_table_[kVopcVCmpClassF32] = 4;
        hybrid_device_perf_table_[kVIVAddF16] = 4;
        hybrid_device_perf_table_[kVIVMulF16] = 4;
        hybrid_device_perf_table_[kVIVMadF16] = 4;
        hybrid_device_perf_table_[kVIVFmaF16] = 4;
        hybrid_device_perf_table_[kVIVDivScaleF16] = 4;
        hybrid_device_perf_table_[kVIVDivFmasF16] = 4;
        hybrid_device_perf_table_[kFra] = 4;
        hybrid_device_perf_table_[kViVFractF16] = 4;
        hybrid_device_perf_table_[kViVTruncF16] = 4;
        hybrid_device_perf_table_[kViVCeilF16] = 4;
        hybrid_device_perf_table_[kViVRndneF16] = 4;
        hybrid_device_perf_table_[kViVFloorF16] = 4;
        hybrid_device_perf_table_[kViVFrexpMantF16] = 4;
        hybrid_device_perf_table_[kViVFrexpExpI16F16] = 4;
        hybrid_device_perf_table_[kViVLdexpF16] = 4;
        hybrid_device_perf_table_[kViVMaxF16] = 4;
        hybrid_device_perf_table_[kViVMinF16] = 4;
        hybrid_device_perf_table_[kViVinterpp1llF16] = 4;
        hybrid_device_perf_table_[kViVinterpp1lvF16] = 4;
        hybrid_device_perf_table_[kViVinterpp2F16] = 4;
        hybrid_device_perf_table_[kViVAddU16] = 4;
        hybrid_device_perf_table_[kViVSubI16] = 4;
        hybrid_device_perf_table_[kViVSubU16] = 4;
        hybrid_device_perf_table_[kViVMulI16] = 4;
        hybrid_device_perf_table_[kViVMulU16] = 4;
        hybrid_device_perf_table_[kViVMadI16] = 4;
        hybrid_device_perf_table_[kViVMadU16] = 4;
        hybrid_device_perf_table_[kViVMaxI16] = 4;
        hybrid_device_perf_table_[kViVMinI16] = 4;
        hybrid_device_perf_table_[kViVMaxU16] = 4;
        hybrid_device_perf_table_[kViVMinU16] = 4;
        hybrid_device_perf_table_[kViVAshrI16] = 4;
        hybrid_device_perf_table_[kViVLshrB16] = 4;
        hybrid_device_perf_table_[kViVLshlB16] = 4;
        hybrid_device_perf_table_[kViVCvtI16F16] = 4;
        hybrid_device_perf_table_[kViVCvtU16F16] = 4;
        hybrid_device_perf_table_[kViVCvtF16I16] = 4;
        hybrid_device_perf_table_[kViVCvtF16U16] = 4;
        hybrid_device_perf_table_[kViVCvtPERMB32] = 4;
        hybrid_device_perf_table_[kVop1VFractF32] = 4;
        hybrid_device_perf_table_[kVop1VTruncF32] = 4;
        hybrid_device_perf_table_[kVop2VMaxLegacyF32] = 4;
        hybrid_device_perf_table_[kVop2VMinLegacyF32] = 4;
        hybrid_device_perf_table_[kVop2VMinF32] = 4;
        hybrid_device_perf_table_[kVop2VMaxF32] = 4;
        hybrid_device_perf_table_[kVop2VCeilF32] = 4;
        hybrid_device_perf_table_[kVop2VRndneF32] = 4;
        hybrid_device_perf_table_[kVop2VFloorF32] = 4;
        hybrid_device_perf_table_[kVop2VMinI32] = 4;
        hybrid_device_perf_table_[kVop2VMaxU32] = 4;
        hybrid_device_perf_table_[kVop2VMinU32] = 4;
        hybrid_device_perf_table_[kVop2VASHRI32] = 4;

        // 1/4 full rate.
        hybrid_device_perf_table_[kVop3VDivScaleF32] = 16;
        hybrid_device_perf_table_[kVop3VDivFmasF32] = 16;
        hybrid_device_perf_table_[kVop3VDivFixupF32] = 16;

        // Half rate.
        hybrid_device_perf_table_[kVop1VLogF32] = 8;
        hybrid_device_perf_table_[kVOP1VLogClampF32] = 8;

        // 1/4 half rate.
        hybrid_device_perf_table_[kVop1VCvtF32F64] = 32;
        hybrid_device_perf_table_[kVop1VCvtI32F64] = 32;
        hybrid_device_perf_table_[kVop1VCvtF64I32] = 32;
        hybrid_device_perf_table_[kVop1VCvtU32F64] = 32;
        hybrid_device_perf_table_[kVop1VCvtF64U32] = 32;
        hybrid_device_perf_table_[kVop3VMinF64] = 32;
        hybrid_device_perf_table_[kVop3VMaxF64] = 32;
        hybrid_device_perf_table_[kVop3VLdexPF64] = 32;
        hybrid_device_perf_table_[kVop3VFrexPMantF64] = 32;
        hybrid_device_perf_table_[kVop3VFrexPExpI32F64] = 32;
        hybrid_device_perf_table_[kVop1VFracTF64] = 32;
        hybrid_device_perf_table_[kVop1VTrunCF64] = 32;
        hybrid_device_perf_table_[kVop1VCeilF64] = 32;
        hybrid_device_perf_table_[kVop1VRndnEF64] = 32;
        hybrid_device_perf_table_[kVop1VFlooRF64] = 32;
        hybrid_device_perf_table_[kVop3VAshrI64] = 32;
        hybrid_device_perf_table_[kVop3VDivScaleF64] = 32;
        hybrid_device_perf_table_[kVop3VDivFixupF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpFF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpLtF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpEqF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpLeF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpGtF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpLgF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpGeF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpOF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpUF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpNgeF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpNlgF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpNgtF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpNleF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpNeqF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpNltF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpTruF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxFF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxLtF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxEqF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxLeF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxGtF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxLgF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxGeF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxOF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxUF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxNgeF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxNlgF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxNgtF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxNleF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxNeqF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxNltF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxTruF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpFI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpLtI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpEqI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpLeI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpGtI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpLgI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpGeI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpTI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxFI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxLtI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxEqI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxLeI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxGtI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxLgI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxGeI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxTI64] = 32;
        hybrid_device_perf_table_[kVopcVCmpFU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpLtU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpEqU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpLeU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpGtU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpLgU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpGeU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpTU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxFU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxLtU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxEqU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxLeU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxGtU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxLgU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxGeU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxTU64] = 32;
        hybrid_device_perf_table_[kVopcVCmpClassF64] = 32;
        hybrid_device_perf_table_[kVopcVCmpxClassF64] = 32;

        // Quarter rate.
        hybrid_device_perf_table_[kVOP1VSqrtF32] = 16;
        hybrid_device_perf_table_[kVOP3VMulLoI32] = 16;
        hybrid_device_perf_table_[kVOP3VMulHiI32] = 16;
        hybrid_device_perf_table_[kVOP3VMulLoU32] = 16;
        hybrid_device_perf_table_[kVOP3VMulHiU32] = 16;
        hybrid_device_perf_table_[kVOP3VMadU64U32] = 16;
        hybrid_device_perf_table_[kVOP3VMadI64I32] = 16;
        hybrid_device_perf_table_[kVOP1VExpF32] = 16;
        hybrid_device_perf_table_[kVOP1VLogClampF32] = 16;
        hybrid_device_perf_table_[kVOP1VRcpClampF32] = 16;
        hybrid_device_perf_table_[kVOP1VRcpLegacyF32] = 16;
        hybrid_device_perf_table_[kVOP1VRcpF32] = 16;
        hybrid_device_perf_table_[kVOP1VRcpIFlagF32] = 16;
        hybrid_device_perf_table_[kVOP1VRsqClampF32] = 16;
        hybrid_device_perf_table_[kVOP1VRsqLegacyF32] = 16;
        hybrid_device_perf_table_[kVOP1VRsqF32] = 16;
        hybrid_device_perf_table_[kVOP1VSqrtF32] = 16;
        hybrid_device_perf_table_[kVOP1VSinF32] = 16;
        hybrid_device_perf_table_[kVOP1VCosF32] = 16;
        hybrid_device_perf_table_[kVOP1VRcpF64] = 16;
        hybrid_device_perf_table_[kVOP1VRcpClampF64] = 16;
        hybrid_device_perf_table_[kVOP1VRsqF64] = 16;
        hybrid_device_perf_table_[kVOP1VRsqClampF64] = 16;
        hybrid_device_perf_table_[kVOP3VTrigPreopF64] = 16;
        hybrid_device_perf_table_[kVOP3VMulF64] = 16;
        hybrid_device_perf_table_[kVOP3VFmaF64] = 16;
        hybrid_device_perf_table_[kVOP3VDivFmasF64] = 16;
        hybrid_device_perf_table_[kVOP3VMqsadPkU16U8] = 16;
        hybrid_device_perf_table_[kVOP3VMqsadU32U8] = 16;
        hybrid_device_perf_table_[kViVRcpF16] = 16;
        hybrid_device_perf_table_[kViVSqrtF16] = 16;
        hybrid_device_perf_table_[kViVRsqF16] = 16;
        hybrid_device_perf_table_[kViVExpF16] = 16;
        hybrid_device_perf_table_[kViVLogF16] = 16;
        hybrid_device_perf_table_[kViVSinF16] = 16;
        hybrid_device_perf_table_[kViVCosF16] = 16;
        hybrid_device_perf_table_[kViQSadPkU16U8] = 16;

        // 1/4 quarter rate.
        hybrid_device_perf_table_[kVOP3VMulF64] = 64;
        hybrid_device_perf_table_[kVOP3VFmaF64] = 64;
        hybrid_device_perf_table_[kVop1VSqrtF64] = 64;
        hybrid_device_perf_table_[kVOP1VRsqClampF64] = 64;
        hybrid_device_perf_table_[kVOP1VRsqF64] = 64;
    }
}

void Instruction::SetUpHalfDevicesPerfTables()
{
    // Full-rate.
    half_device_perf_table_[kVop2NoHwImplVMacF32] = 4;
    half_device_perf_table_[kVop2NoHwImplVMacLegacyF32] = 4;
    half_device_perf_table_[kVop2NoHwImplVSubbrevU32] = 4;
    half_device_perf_table_[kVop2NoHwImplVSubF32] = 4;
    half_device_perf_table_[kVop2NoHwImplVSubrevF32] = 4;
    half_device_perf_table_[kVop2NoHwImplVSubrevI32] = 4;
    half_device_perf_table_[kVop2NoHwImplVMadmkF32] = 4;
    half_device_perf_table_[kVop2NoHwImplVMadakF32] = 4;
    half_device_perf_table_[kVop2NoHwImplVSubbU32] = 4;
    half_device_perf_table_[kVop2NoHwImplVLshrrevB32] = 4;
    half_device_perf_table_[kVop2NoHwImplVAshrrevI32] = 4;
    half_device_perf_table_[kVop2NoHwImplVLshlrevB32] = 4;
    half_device_perf_table_[kVop1VMovB32] = 4;
    half_device_perf_table_[kVop2VMovFedB32] = 4;
    half_device_perf_table_[kVop1VNotB32] = 4;
    half_device_perf_table_[kVop1VCvtF32I32] = 4;
    half_device_perf_table_[kVop1VCvtF32U32] = 4;
    half_device_perf_table_[kVop1VCvtU32F32] = 4;
    half_device_perf_table_[kVop1VCvtI32F32] = 4;
    half_device_perf_table_[kVop1VCvtRpiI32F32] = 4;
    half_device_perf_table_[kVop1VCvtFlrI32F32] = 4;
    half_device_perf_table_[kVop1VCvtOffF32I4] = 4;
    half_device_perf_table_[kVop1VBfrevB32] = 4;
    half_device_perf_table_[kVop2VMulI32I24] = 4;
    half_device_perf_table_[kVop2VMulHII32I24] = 4;
    half_device_perf_table_[kVop2VMulU32U24] = 4;
    half_device_perf_table_[kVop2VMulHIU32U24] = 4;
    half_device_perf_table_[kVop2VMulF32] = 4;
    half_device_perf_table_[kVop2VMulLegacyF32] = 4;
    half_device_perf_table_[kVop2VAddF32] = 4;
    half_device_perf_table_[kVop2VAddI32] = 4;
    half_device_perf_table_[kVop2VAddcU32] = 4;
    hybrid_device_perf_table_[kVop2VSubI32] = 4;
    half_device_perf_table_[kVop2NodocVAddU32] = 4;
    half_device_perf_table_[kVop2NodocVSubU32] = 4;
    half_device_perf_table_[kVop2VLshLB32] = 4;
    half_device_perf_table_[kVop2VXorB32] = 4;
    half_device_perf_table_[kVop2VOrB32] = 4;
    half_device_perf_table_[kVop2VAndB32] = 4;
    half_device_perf_table_[kVop3VMadLegacyF32] = 4;
    half_device_perf_table_[kVop3VFmaF32] = 4;
    half_device_perf_table_[kVop3VMulLitF32] = 4;
    half_device_perf_table_[kVop3VAddF64] = 4;
    half_device_perf_table_[kVop3VMadI32I24] = 4;
    half_device_perf_table_[kVop3VMadU32U24] = 4;
    half_device_perf_table_[kVop3VMadF32] = 4;
    half_device_perf_table_[kVop2VBfeI32] = 4;
    half_device_perf_table_[kVop3VAlignbitB32] = 4;
    half_device_perf_table_[kVop3VAlignbyteB32] = 4;
    half_device_perf_table_[kVop2VBfmB32] = 4;
    half_device_perf_table_[kVop3VBfiB32] = 4;
    half_device_perf_table_[kVop2VBcntU32B32] = 4;
    half_device_perf_table_[kVop2VMbcntLoU32B32] = 4;
    half_device_perf_table_[kVop2VMbcntHiU32B32] = 4;
    half_device_perf_table_[kVop1VFfbhU32] = 4;
    half_device_perf_table_[kVop1VFfblB32] = 4;
    half_device_perf_table_[kVop1VFfbhI32] = 4;
    half_device_perf_table_[kVop3VSadU8] = 4;
    half_device_perf_table_[kVop3VMSadU8] = 4;
    half_device_perf_table_[kVop3VSADHiU8] = 4;
    half_device_perf_table_[kVop3VSADU16] = 4;
    half_device_perf_table_[kVop3VSadU32] = 4;
    half_device_perf_table_[kVop3VLerpU8] = 4;
    half_device_perf_table_[kVop1VCvtF32Ubyte0] = 4;
    half_device_perf_table_[kVop1VCvtF32Ubyte1] = 4;
    half_device_perf_table_[kVop1VCvtF32Ubyte2] = 4;
    half_device_perf_table_[kVop1VCvtF32Ubyte3] = 4;
    half_device_perf_table_[kVop1VCvtF32F16] = 4;
    half_device_perf_table_[kVop1VCvtOffF32I4] = 4;
    half_device_perf_table_[kVop1VCvtF16F32] = 4;
    half_device_perf_table_[kVop2VCvtPkrtzF16F32] = 4;
    half_device_perf_table_[kVop2VCvtPknormI16F32] = 4;
    half_device_perf_table_[kVop2VCvtPknormU16F32] = 4;
    half_device_perf_table_[kVop2VCvtPkU16U32] = 4;
    half_device_perf_table_[kVop2VCvtPkI16I32] = 4;
    half_device_perf_table_[kVop3VFmaF32] = 4;
    half_device_perf_table_[kVop2VMulU32U24] = 4;
    half_device_perf_table_[kVop2VMulHIU32U24] = 4;
    half_device_perf_table_[kVop3VMadU32U24] = 4;
    half_device_perf_table_[kVop3VMulI32I24] = 4;
    half_device_perf_table_[kVop2VMulHII32I24] = 4;
    half_device_perf_table_[kVop3VMadI32I24] = 4;
    half_device_perf_table_[kVop3VMulLitF32] = 4;
    half_device_perf_table_[kVop2VCndmaskB32] = 4;
    half_device_perf_table_[kVop3VCubeidF32] = 4;
    half_device_perf_table_[kVop3VCubescF32] = 4;
    half_device_perf_table_[kVop3VCubetcF32] = 4;
    half_device_perf_table_[kVop3VCubemaF32] = 4;
    half_device_perf_table_[kVop3VMaxF32] = 4;
    half_device_perf_table_[kVop3VMaxI32] = 4;
    half_device_perf_table_[kVop3VMaxU32] = 4;
    half_device_perf_table_[kVop3VMin3F32] = 4;
    half_device_perf_table_[kVop3VMin3I32] = 4;
    half_device_perf_table_[kVop3VMin3U32] = 4;
    half_device_perf_table_[kVop3VMed3F32] = 4;
    half_device_perf_table_[kVop3VMed3I32] = 4;
    half_device_perf_table_[kVop3VMed3U32] = 4;
    half_device_perf_table_[kVop3VCvtpkU8F32] = 4;
    half_device_perf_table_[kVop1VFRexpMantF32] = 4;
    half_device_perf_table_[kVop1VFRexpExpI32F32] = 4;
    half_device_perf_table_[kVop2VLDexpF32] = 4;
    half_device_perf_table_[kVopcVCmpeqF32] = 4;
    half_device_perf_table_[kVopcVCmpleF32] = 4;
    half_device_perf_table_[kVopcVCmpgtF32] = 4;
    half_device_perf_table_[kVopcVCmplgF32] = 4;
    half_device_perf_table_[kVopcVCmpgeF32] = 4;
    half_device_perf_table_[kVopcVCmpOF32] = 4;
    half_device_perf_table_[kVopcVCmpUF32] = 4;
    half_device_perf_table_[kVopcVCmpNgeF32] = 4;
    half_device_perf_table_[kVopcVCmpNlgF32] = 4;
    half_device_perf_table_[kVopcVCmpNgtF32] = 4;
    half_device_perf_table_[kVopcVCmpNleF32] = 4;
    half_device_perf_table_[kVopcVCmpNeqF32] = 4;
    half_device_perf_table_[kVopcVCmpNltF32] = 4;
    half_device_perf_table_[kVopcVCmpTruF32] = 4;
    half_device_perf_table_[kVopcVCmpNgeF32] = 4;
    half_device_perf_table_[kVopcVCmpNlgF32] = 4;
    half_device_perf_table_[kVopcVCmpNgtF32] = 4;
    half_device_perf_table_[kVopcVCmpNleF32] = 4;
    half_device_perf_table_[kVopcVCmpNeqF32] = 4;
    half_device_perf_table_[kVopcVCmpNltF32] = 4;
    half_device_perf_table_[kVopcVCmpTruF32] = 4;
    half_device_perf_table_[kVopcVCmpFI32] = 4;
    half_device_perf_table_[kVopcVCmpLtI32] = 4;
    half_device_perf_table_[kVopcVCmpEqI32] = 4;
    half_device_perf_table_[kVopcVCmpLeI32] = 4;
    half_device_perf_table_[kVopcVCmpGtI32] = 4;
    half_device_perf_table_[kVopcVCmpNeI32] = 4;
    half_device_perf_table_[kVopcVCmpGeI32] = 4;
    half_device_perf_table_[kVopcVCmpTI32] = 4;
    half_device_perf_table_[kVopcVCmpxFI32] = 4;
    half_device_perf_table_[kVopcVCmpxLtI32] = 4;
    half_device_perf_table_[kVopcVCmpxEqI32] = 4;
    half_device_perf_table_[kVopcVCmpxLeI32] = 4;
    half_device_perf_table_[kVopcVCmpxGtI32] = 4;
    half_device_perf_table_[kVopcVCmpxNeI32] = 4;
    half_device_perf_table_[kVopcVCmpxGeI32] = 4;
    half_device_perf_table_[kVopcVCmpxTI32] = 4;
    half_device_perf_table_[kVopcVCmpFU32] = 4;
    half_device_perf_table_[kVopcVCmpLtU32] = 4;
    half_device_perf_table_[kVopcVCmpEqU32] = 4;
    half_device_perf_table_[kVopcVCmpLeU32] = 4;
    half_device_perf_table_[kVopcVCmpGtU32] = 4;
    half_device_perf_table_[kVopcVCmpNeU32] = 4;
    half_device_perf_table_[kVopcVCmpGeU32] = 4;
    half_device_perf_table_[kVopcVCmpTU32] = 4;
    half_device_perf_table_[kVopcVCmpxFU32] = 4;
    half_device_perf_table_[kVopcVCmpxLtU32] = 4;
    half_device_perf_table_[kVopcVCmpxEqU32] = 4;
    half_device_perf_table_[kVopcVCmpxLeU32] = 4;
    half_device_perf_table_[kVopcVCmpxGtU32] = 4;
    half_device_perf_table_[kVopcVCmpxNeU32] = 4;
    half_device_perf_table_[kVopcVCmpxGeU32] = 4;
    half_device_perf_table_[kVopcVCmpxTU32] = 4;
    half_device_perf_table_[kVopcVCmpClassF32] = 4;
    half_device_perf_table_[kVIVAddF16] = 4;
    half_device_perf_table_[kVIVMulF16] = 4;
    half_device_perf_table_[kVIVMadF16] = 4;
    half_device_perf_table_[kVIVFmaF16] = 4;
    half_device_perf_table_[kVIVDivScaleF16] = 4;
    half_device_perf_table_[kVIVDivFmasF16] = 4;
    half_device_perf_table_[kFra] = 4;
    half_device_perf_table_[kViVFractF16] = 4;
    half_device_perf_table_[kViVTruncF16] = 4;
    half_device_perf_table_[kViVCeilF16] = 4;
    half_device_perf_table_[kViVRndneF16] = 4;
    half_device_perf_table_[kViVFloorF16] = 4;
    half_device_perf_table_[kViVFrexpMantF16] = 4;
    half_device_perf_table_[kViVFrexpExpI16F16] = 4;
    half_device_perf_table_[kViVLdexpF16] = 4;
    half_device_perf_table_[kViVMaxF16] = 4;
    half_device_perf_table_[kViVMinF16] = 4;
    half_device_perf_table_[kViVinterpp1llF16] = 4;
    half_device_perf_table_[kViVinterpp1lvF16] = 4;
    half_device_perf_table_[kViVinterpp2F16] = 4;
    half_device_perf_table_[kViVAddU16] = 4;
    half_device_perf_table_[kViVSubI16] = 4;
    half_device_perf_table_[kViVSubU16] = 4;
    half_device_perf_table_[kViVMulI16] = 4;
    half_device_perf_table_[kViVMulU16] = 4;
    half_device_perf_table_[kViVMadI16] = 4;
    half_device_perf_table_[kViVMadU16] = 4;
    half_device_perf_table_[kViVMaxI16] = 4;
    half_device_perf_table_[kViVMinI16] = 4;
    half_device_perf_table_[kViVMaxU16] = 4;
    half_device_perf_table_[kViVMinU16] = 4;
    half_device_perf_table_[kViVAshrI16] = 4;
    half_device_perf_table_[kViVLshrB16] = 4;
    half_device_perf_table_[kViVLshlB16] = 4;
    half_device_perf_table_[kViVCvtI16F16] = 4;
    half_device_perf_table_[kViVCvtU16F16] = 4;
    half_device_perf_table_[kViVCvtF16I16] = 4;
    half_device_perf_table_[kViVCvtF16U16] = 4;
    half_device_perf_table_[kViVCvtPERMB32] = 4;
    half_device_perf_table_[kVop3VDivScaleF32] = 4;
    half_device_perf_table_[kVop3VDivFmasF32] = 4;
    half_device_perf_table_[kVop3VDivFixupF32] = 4;
    half_device_perf_table_[kVop1VFractF32] = 4;
    half_device_perf_table_[kVop1VTruncF32] = 4;
    half_device_perf_table_[kVop2VMaxLegacyF32] = 4;
    half_device_perf_table_[kVop2VMinLegacyF32] = 4;
    half_device_perf_table_[kVop2VMinF32] = 4;
    half_device_perf_table_[kVop2VMaxF32] = 4;
    half_device_perf_table_[kVop2VCeilF32] = 4;
    half_device_perf_table_[kVop2VRndneF32] = 4;
    half_device_perf_table_[kVop2VFloorF32] = 4;
    half_device_perf_table_[kVop2VMinI32] = 4;
    half_device_perf_table_[kVop2VMaxU32] = 4;
    half_device_perf_table_[kVop2VMinU32] = 4;
    half_device_perf_table_[kVop2VASHRI32] = 4;

    // Half rate.
    half_device_perf_table_[kVop1VLogF32] = 8;
    half_device_perf_table_[kVOP1VLogClampF32] = 8;
    half_device_perf_table_[kVop1VCvtF32F64] = 8;
    half_device_perf_table_[kVop1VCvtI32F64] = 8;
    half_device_perf_table_[kVop1VCvtF64I32] = 8;
    half_device_perf_table_[kVop1VCvtU32F64] = 8;
    half_device_perf_table_[kVop1VCvtF64U32] = 8;
    half_device_perf_table_[kVop3VMinF64] = 8;
    half_device_perf_table_[kVop3VMaxF64] = 8;
    half_device_perf_table_[kVop3VLdexPF64] = 8;
    half_device_perf_table_[kVop3VFrexPMantF64] = 8;
    half_device_perf_table_[kVop3VFrexPExpI32F64] = 8;
    half_device_perf_table_[kVop1VFracTF64] = 8;
    half_device_perf_table_[kVop1VTrunCF64] = 8;
    half_device_perf_table_[kVop1VCeilF64] = 8;
    half_device_perf_table_[kVop1VRndnEF64] = 8;
    half_device_perf_table_[kVop1VFlooRF64] = 8;
    half_device_perf_table_[kVop3VAshrI64] = 8;
    half_device_perf_table_[kVop3VDivScaleF64] = 8;
    half_device_perf_table_[kVop3VDivFixupF64] = 8;
    half_device_perf_table_[kVopcVCmpFF64] = 8;
    half_device_perf_table_[kVopcVCmpLtF64] = 8;
    half_device_perf_table_[kVopcVCmpEqF64] = 8;
    half_device_perf_table_[kVopcVCmpLeF64] = 8;
    half_device_perf_table_[kVopcVCmpGtF64] = 8;
    half_device_perf_table_[kVopcVCmpLgF64] = 8;
    half_device_perf_table_[kVopcVCmpGeF64] = 8;
    half_device_perf_table_[kVopcVCmpOF64] = 8;
    half_device_perf_table_[kVopcVCmpUF64] = 8;
    half_device_perf_table_[kVopcVCmpNgeF64] = 8;
    half_device_perf_table_[kVopcVCmpNlgF64] = 8;
    half_device_perf_table_[kVopcVCmpNgtF64] = 8;
    half_device_perf_table_[kVopcVCmpNleF64] = 8;
    half_device_perf_table_[kVopcVCmpNeqF64] = 8;
    half_device_perf_table_[kVopcVCmpNltF64] = 8;
    half_device_perf_table_[kVopcVCmpTruF64] = 8;
    half_device_perf_table_[kVopcVCmpxFF64] = 8;
    half_device_perf_table_[kVopcVCmpxLtF64] = 8;
    half_device_perf_table_[kVopcVCmpxEqF64] = 8;
    half_device_perf_table_[kVopcVCmpxLeF64] = 8;
    half_device_perf_table_[kVopcVCmpxGtF64] = 8;
    half_device_perf_table_[kVopcVCmpxLgF64] = 8;
    half_device_perf_table_[kVopcVCmpxGeF64] = 8;
    half_device_perf_table_[kVopcVCmpxOF64] = 8;
    half_device_perf_table_[kVopcVCmpxUF64] = 8;
    half_device_perf_table_[kVopcVCmpxNgeF64] = 8;
    half_device_perf_table_[kVopcVCmpxNlgF64] = 8;
    half_device_perf_table_[kVopcVCmpxNgtF64] = 8;
    half_device_perf_table_[kVopcVCmpxNleF64] = 8;
    half_device_perf_table_[kVopcVCmpxNeqF64] = 8;
    half_device_perf_table_[kVopcVCmpxNltF64] = 8;
    half_device_perf_table_[kVopcVCmpxTruF64] = 8;
    half_device_perf_table_[kVopcVCmpFI64] = 8;
    half_device_perf_table_[kVopcVCmpLtI64] = 8;
    half_device_perf_table_[kVopcVCmpEqI64] = 8;
    half_device_perf_table_[kVopcVCmpLeI64] = 8;
    half_device_perf_table_[kVopcVCmpGtI64] = 8;
    half_device_perf_table_[kVopcVCmpLgI64] = 8;
    half_device_perf_table_[kVopcVCmpGeI64] = 8;
    half_device_perf_table_[kVopcVCmpTI64] = 8;
    half_device_perf_table_[kVopcVCmpxFI64] = 8;
    half_device_perf_table_[kVopcVCmpxLtI64] = 8;
    half_device_perf_table_[kVopcVCmpxEqI64] = 8;
    half_device_perf_table_[kVopcVCmpxLeI64] = 8;
    half_device_perf_table_[kVopcVCmpxGtI64] = 8;
    half_device_perf_table_[kVopcVCmpxLgI64] = 8;
    half_device_perf_table_[kVopcVCmpxGeI64] = 8;
    half_device_perf_table_[kVopcVCmpxTI64] = 8;
    half_device_perf_table_[kVopcVCmpFU64] = 8;
    half_device_perf_table_[kVopcVCmpLtU64] = 8;
    half_device_perf_table_[kVopcVCmpEqU64] = 8;
    half_device_perf_table_[kVopcVCmpLeU64] = 8;
    half_device_perf_table_[kVopcVCmpGtU64] = 8;
    half_device_perf_table_[kVopcVCmpLgU64] = 8;
    half_device_perf_table_[kVopcVCmpGeU64] = 8;
    half_device_perf_table_[kVopcVCmpTU64] = 8;
    half_device_perf_table_[kVopcVCmpxFU64] = 8;
    half_device_perf_table_[kVopcVCmpxLtU64] = 8;
    half_device_perf_table_[kVopcVCmpxEqU64] = 8;
    half_device_perf_table_[kVopcVCmpxLeU64] = 8;
    half_device_perf_table_[kVopcVCmpxGtU64] = 8;
    half_device_perf_table_[kVopcVCmpxLgU64] = 8;
    half_device_perf_table_[kVopcVCmpxGeU64] = 8;
    half_device_perf_table_[kVopcVCmpxTU64] = 8;
    half_device_perf_table_[kVopcVCmpClassF64] = 8;
    half_device_perf_table_[kVopcVCmpxClassF64] = 8;

    // 32-bit integer mul, muladd, and double precision
    // mul/fmas.
    half_device_perf_table_[kVOP3VMadU64U32] = 8;
    half_device_perf_table_[kVOP3VMulLoI32] = 8;
    half_device_perf_table_[kVOP3VMulHiI32] = 8;
    half_device_perf_table_[kVOP3VMulLoU32] = 8;
    half_device_perf_table_[kVOP3VMulHiU32] = 8;
    half_device_perf_table_[kVOP3VMadI64I32] = 8;
    half_device_perf_table_[kVOP3VMulF64] = 8;
    half_device_perf_table_[kVOP3VFmaF64] = 8;
    half_device_perf_table_[kVOP3VDivFmasF64] = 8;
    half_device_perf_table_[kVOP3VMulF64] = 8;
    half_device_perf_table_[kVOP3VFmaF64] = 8;

    // Quarter rate.
    half_device_perf_table_[kVOP1VSqrtF32] = 16;
    half_device_perf_table_[kVOP1VExpF32] = 16;
    half_device_perf_table_[kVOP1VLogClampF32] = 16;
    half_device_perf_table_[kVOP1VRcpClampF32] = 16;
    half_device_perf_table_[kVOP1VRcpLegacyF32] = 16;
    half_device_perf_table_[kVOP1VRcpF32] = 16;
    half_device_perf_table_[kVOP1VRcpIFlagF32] = 16;
    half_device_perf_table_[kVOP1VRsqClampF32] = 16;
    half_device_perf_table_[kVOP1VRsqLegacyF32] = 16;
    half_device_perf_table_[kVOP1VRsqF32] = 16;
    half_device_perf_table_[kVOP1VSqrtF32] = 16;
    half_device_perf_table_[kVOP1VSinF32] = 16;
    half_device_perf_table_[kVOP1VCosF32] = 16;
    half_device_perf_table_[kVOP1VRcpF64] = 16;
    half_device_perf_table_[kVOP1VRcpClampF64] = 16;
    half_device_perf_table_[kVOP1VRsqF64] = 16;
    half_device_perf_table_[kVOP1VRsqClampF64] = 16;
    half_device_perf_table_[kVOP3VTrigPreopF64] = 16;
    half_device_perf_table_[kVOP3VMqsadPkU16U8] = 16;
    half_device_perf_table_[kVOP3VMqsadU32U8] = 16;
    half_device_perf_table_[kViVRcpF16] = 16;
    half_device_perf_table_[kViVSqrtF16] = 16;
    half_device_perf_table_[kViVRsqF16] = 16;
    half_device_perf_table_[kViVExpF16] = 16;
    half_device_perf_table_[kViVLogF16] = 16;
    half_device_perf_table_[kViVSinF16] = 16;
    half_device_perf_table_[kViVCosF16] = 16;
    half_device_perf_table_[kViQSadPkU16U8] = 16;
    half_device_perf_table_[kVop1VSqrtF64] = 16;
    half_device_perf_table_[kVOP1VRsqClampF64] = 16;
    half_device_perf_table_[kVOP1VRsqF64] = 16;
}

void Instruction::SetUpScalarPerfTables()
{
    if (scalar_device_perf_table_.empty())
    {
        scalar_device_perf_table_[kSop2SAddU32] = 4;
        scalar_device_perf_table_[kSop2SSubU32] = 4;
        scalar_device_perf_table_[kSop2SAddI32] = 4;
        scalar_device_perf_table_[kSop2SSubI32] = 4;
        scalar_device_perf_table_[kSop2SAddcU32] = 4;
        scalar_device_perf_table_[kSop2SSubbU32] = 4;
        scalar_device_perf_table_[kSop2SLshlB32] = 4;
        scalar_device_perf_table_[kSop2SLshlB64] = 4;
        scalar_device_perf_table_[kSop2SLshrB32] = 4;
        scalar_device_perf_table_[kSop2SLshrB64] = 4;
        scalar_device_perf_table_[kSop2SMulI32] = 4;
        scalar_device_perf_table_[kSop2SAndB32] = 4;
        scalar_device_perf_table_[kSop2SAndB64] = 4;
        scalar_device_perf_table_[kSop2SOrB32] = 4;
        scalar_device_perf_table_[kSop2SOrB64] = 4;
        scalar_device_perf_table_[kSop2SXorB32] = 4;
        scalar_device_perf_table_[kSop2SXorB64] = 4;
        scalar_device_perf_table_[kSop2SAndn2B32] = 4;
        scalar_device_perf_table_[kSop2SAndn2B64] = 4;
        scalar_device_perf_table_[kSop2SOrn2B32] = 4;
        scalar_device_perf_table_[kSop2SOrn2B64] = 4;
        scalar_device_perf_table_[kSop2SNandB32] = 4;
        scalar_device_perf_table_[kSop2SNandB64] = 4;
        scalar_device_perf_table_[kSop2SNorB32] = 4;
        scalar_device_perf_table_[kSop2SNorB64] = 4;
        scalar_device_perf_table_[kSop2SXnorB32] = 4;
        scalar_device_perf_table_[kSop2SXnorB64] = 4;
        scalar_device_perf_table_[kSop1SMovB32] = 4;
        scalar_device_perf_table_[kSop1SMovB64] = 4;
        scalar_device_perf_table_[kSop1SCmovB32] = 4;
        scalar_device_perf_table_[kSop1SCmovB64] = 4;
        scalar_device_perf_table_[kSop1SNotB32] = 4;
        scalar_device_perf_table_[kSop1SNotB64] = 4;

        scalar_device_perf_table_[kSopcSCmpEqI32] = 4;
        scalar_device_perf_table_[kSopcSCmpNeI32] = 4;
        scalar_device_perf_table_[kSopcSCmpGtI32] = 4;
        scalar_device_perf_table_[kSopcSCmpGeI32] = 4;
        scalar_device_perf_table_[kSopcSCmpLeI32] = 4;
        scalar_device_perf_table_[kSopcSCmpLtI32] = 4;
        scalar_device_perf_table_[kSopcSCmpEqU32] = 4;
        scalar_device_perf_table_[kSopcSCmpNeU32] = 4;
        scalar_device_perf_table_[kSopcSCmpGtU32] = 4;
        scalar_device_perf_table_[kSopcSCmpGeU32] = 4;
        scalar_device_perf_table_[kSopcSCmpLeU32] = 4;
        scalar_device_perf_table_[kSopcSCmpLtU32] = 4;
        scalar_device_perf_table_[kSopkSCmpkEqI32] = 4;
        scalar_device_perf_table_[kSopkSCmpkNeI32] = 4;
        scalar_device_perf_table_[kSopkSCmpkGtI32] = 4;
        scalar_device_perf_table_[kSopkSCmpkGeI32] = 4;
        scalar_device_perf_table_[kSopkSCmpkLeI32] = 4;
        scalar_device_perf_table_[kSopkSCmpkLtI32] = 4;
        scalar_device_perf_table_[kSopkSCmpkEqU32] = 4;
        scalar_device_perf_table_[kSopkSCmpkNeU32] = 4;
        scalar_device_perf_table_[kSopkSCmpkGtU32] = 4;
        scalar_device_perf_table_[kSopkSCmpkGeU32] = 4;
        scalar_device_perf_table_[kSopkSCmpkLeU32] = 4;
        scalar_device_perf_table_[kSopkSCmpkLtU32] = 4;
        scalar_device_perf_table_[kSopcSBitcmp0B32] = 4;
        scalar_device_perf_table_[kSopcSBitcmp0B64] = 4;
        scalar_device_perf_table_[kSopcSBItcmp1B32] = 4;
        scalar_device_perf_table_[kSopcSBItcmp1B64] = 4;
        scalar_device_perf_table_[kSop2SMovkI32] = 4;
        scalar_device_perf_table_[kSop2SAshrI32] = 4;
        scalar_device_perf_table_[kSop2SAshrI64] = 4;
        scalar_device_perf_table_[kSop2SBfmB32] = 4;
        scalar_device_perf_table_[kSop2SBfmB64] = 4;
        scalar_device_perf_table_[kSop2SBfeI32] = 4;
        scalar_device_perf_table_[kSop2SBfeI64] = 4;
        scalar_device_perf_table_[kSop2SBfeU32] = 4;
        scalar_device_perf_table_[kSop2SBfeU64] = 4;
        scalar_device_perf_table_[kSop1SWqmB32] = 4;
        scalar_device_perf_table_[kSop1SWqmB64] = 4;
        scalar_device_perf_table_[kSop1SQuadmaskB32] = 4;
        scalar_device_perf_table_[kSop1SQuadmaskB64] = 4;
        scalar_device_perf_table_[kSop1SBrevB32] = 4;
        scalar_device_perf_table_[kSop1SBrevB64] = 4;
        scalar_device_perf_table_[kSop1SBCnt0I32B32] = 4;
        scalar_device_perf_table_[kSop1SBCnt0I32B64] = 4;
        scalar_device_perf_table_[kSop1SBcnt1I32B32] = 4;
        scalar_device_perf_table_[kSop1SBcnt1I32B64] = 4;
        scalar_device_perf_table_[kSop1SFf0I32B32] = 4;
        scalar_device_perf_table_[kSop1SFf0I32B64] = 4;
        scalar_device_perf_table_[kSop1SFf1I32B32] = 4;
        scalar_device_perf_table_[kSop1SFf1I32B64] = 4;
        scalar_device_perf_table_[kSop1SFlbitI32B32] = 4;
        scalar_device_perf_table_[kSop1SFlbitI32B64] = 4;
        scalar_device_perf_table_[kSop1SFlbitI32] = 4;
        scalar_device_perf_table_[kSop1SFlbitI32I64] = 4;
        scalar_device_perf_table_[kSop1SBitset0B32] = 4;
        scalar_device_perf_table_[kSop1SBitset0B64] = 4;
        scalar_device_perf_table_[kSop1SBitset1B32] = 4;
        scalar_device_perf_table_[kSop1SBitset1B64] = 4;
        scalar_device_perf_table_[kSop1SAndSaveexecB64] = 4;
        scalar_device_perf_table_[kSop1SOrSaveexecB64] = 4;
        scalar_device_perf_table_[kSop1SXorSaveexecB64] = 4;
        scalar_device_perf_table_[kSop1SAndn2SaveexecB64] = 4;
        scalar_device_perf_table_[kSop1SOrn2SaveexecB64] = 4;
        scalar_device_perf_table_[kSop1SNandSaveexecB64] = 4;
        scalar_device_perf_table_[kSop1SNorSaveexecB64] = 4;
        scalar_device_perf_table_[kSop1SXnorSaveexecB64] = 4;
        scalar_device_perf_table_[kSop1SMovrelsB32] = 4;
        scalar_device_perf_table_[kSop1SMovrelsB64] = 4;
        scalar_device_perf_table_[kSop1SMovreldB32] = 4;
        scalar_device_perf_table_[kSop1SmovreldB64] = 4;
        scalar_device_perf_table_[kSopkSGetregB32] = 4;
        scalar_device_perf_table_[kSopkSSetregB32] = 4;
        scalar_device_perf_table_[kSopkSSetregimm32B32] = 4;
        scalar_device_perf_table_[kSop2SCselectB32] = 4;
        scalar_device_perf_table_[kSop2SCselectB64] = 4;
        scalar_device_perf_table_[kSopkSCMovkI32] = 4;
        scalar_device_perf_table_[kSop1SCmovB32] = 4;
        scalar_device_perf_table_[kSop1SCmovB64] = 4;
        scalar_device_perf_table_[kSop2SAbsdiffI32] = 4;
        scalar_device_perf_table_[kSop2SMinI32] = 4;
        scalar_device_perf_table_[kSop2SMinU32] = 4;
        scalar_device_perf_table_[kSop2SMaxI32] = 4;
        scalar_device_perf_table_[kSop2SMaxU32] = 4;
        scalar_device_perf_table_[kSop1SAbsI32] = 4;
        scalar_device_perf_table_[kSop1SSextI32I8] = 4;
        scalar_device_perf_table_[kSop1SSextI32I16] = 4;

        scalar_device_perf_table_[kSoppSNop] = 1;
        scalar_device_perf_table_[kSoppSEndpgm] = 1;
        scalar_device_perf_table_[kSoppSTrap] = 1;
        scalar_device_perf_table_[kSoppSRfe] = 1;
        scalar_device_perf_table_[kSoppSSetprio] = 1;
        scalar_device_perf_table_[kSoppSSleep] = 1;
        scalar_device_perf_table_[kSoppSSendmsg] = 1;
        scalar_device_perf_table_[kSoppSBarrier] = 1;
        scalar_device_perf_table_[kSoppSSethalt] = 1;
        scalar_device_perf_table_[kSoppSSendmsghalt] = 1;
        scalar_device_perf_table_[kSoppSIcache_inv] = 1;
        scalar_device_perf_table_[kSoppSIncperflevel] = 1;
        scalar_device_perf_table_[kSoppSDecperflevel] = 1;
        scalar_device_perf_table_[kSoppSTtracedata] = 1;
        scalar_device_perf_table_[kSoppSSetpc] = 1;
        scalar_device_perf_table_[kSoppSSwappc] = 1;
    }
}

void Instruction::SetUpQuarterDevicesPerfTables()
{
    // Full-rate.
    quarter_device_perf_table_[kVop2NoHwImplVMacF32] = 4;
    quarter_device_perf_table_[kVop2NoHwImplVMacLegacyF32] = 4;
    quarter_device_perf_table_[kVop2NoHwImplVSubbrevU32] = 4;
    quarter_device_perf_table_[kVop2NoHwImplVSubF32] = 4;
    quarter_device_perf_table_[kVop2NoHwImplVSubrevF32] = 4;
    quarter_device_perf_table_[kVop2NoHwImplVSubrevI32] = 4;
    quarter_device_perf_table_[kVop2NoHwImplVMadmkF32] = 4;
    quarter_device_perf_table_[kVop2NoHwImplVMadakF32] = 4;
    quarter_device_perf_table_[kVop2NoHwImplVSubbU32] = 4;
    quarter_device_perf_table_[kVop2NoHwImplVLshrrevB32] = 4;
    quarter_device_perf_table_[kVop2NoHwImplVAshrrevI32] = 4;
    quarter_device_perf_table_[kVop2NoHwImplVLshlrevB32] = 4;
    quarter_device_perf_table_[kVop1VMovB32] = 4;
    quarter_device_perf_table_[kVop2VMovFedB32] = 4;
    quarter_device_perf_table_[kVop1VNotB32] = 4;
    quarter_device_perf_table_[kVop1VCvtF32I32] = 4;
    quarter_device_perf_table_[kVop1VCvtF32U32] = 4;
    quarter_device_perf_table_[kVop1VCvtU32F32] = 4;
    quarter_device_perf_table_[kVop1VCvtI32F32] = 4;
    quarter_device_perf_table_[kVop1VCvtRpiI32F32] = 4;
    quarter_device_perf_table_[kVop1VCvtFlrI32F32] = 4;
    quarter_device_perf_table_[kVop1VCvtOffF32I4] = 4;
    quarter_device_perf_table_[kVop1VBfrevB32] = 4;
    quarter_device_perf_table_[kVop2VMulI32I24] = 4;
    quarter_device_perf_table_[kVop2VMulHII32I24] = 4;
    quarter_device_perf_table_[kVop2VMulU32U24] = 4;
    quarter_device_perf_table_[kVop2VMulHIU32U24] = 4;
    quarter_device_perf_table_[kVop2VMulF32] = 4;
    quarter_device_perf_table_[kVop2VMulLegacyF32] = 4;
    quarter_device_perf_table_[kVop2VAddF32] = 4;
    quarter_device_perf_table_[kVop2VAddI32] = 4;
    quarter_device_perf_table_[kVop2VAddcU32] = 4;
    hybrid_device_perf_table_ [kVop2VSubI32] = 4;
    quarter_device_perf_table_[kVop2NodocVAddU32] = 4;
    quarter_device_perf_table_[kVop2NodocVSubU32] = 4;
    quarter_device_perf_table_[kVop2VLshLB32] = 4;
    quarter_device_perf_table_[kVop2VXorB32] = 4;
    quarter_device_perf_table_[kVop2VOrB32] = 4;
    quarter_device_perf_table_[kVop2VAndB32] = 4;
    quarter_device_perf_table_[kVop3VMadLegacyF32] = 4;
    quarter_device_perf_table_[kVop3VFmaF32] = 4;
    quarter_device_perf_table_[kVop3VMulLitF32] = 4;
    quarter_device_perf_table_[kVop3VAddF64] = 4;
    quarter_device_perf_table_[kVop3VMadI32I24] = 4;
    quarter_device_perf_table_[kVop3VMadU32U24] = 4;
    quarter_device_perf_table_[kVop3VMadF32] = 4;
    quarter_device_perf_table_[kVop2VBfeI32] = 4;
    quarter_device_perf_table_[kVop3VAlignbitB32] = 4;
    quarter_device_perf_table_[kVop3VAlignbyteB32] = 4;
    quarter_device_perf_table_[kVop2VBfmB32] = 4;
    quarter_device_perf_table_[kVop3VBfiB32] = 4;
    quarter_device_perf_table_[kVop2VBcntU32B32] = 4;
    quarter_device_perf_table_[kVop2VMbcntLoU32B32] = 4;
    quarter_device_perf_table_[kVop2VMbcntHiU32B32] = 4;
    quarter_device_perf_table_[kVop1VFfbhU32] = 4;
    quarter_device_perf_table_[kVop1VFfblB32] = 4;
    quarter_device_perf_table_[kVop1VFfbhI32] = 4;
    quarter_device_perf_table_[kVop3VSadU8] = 4;
    quarter_device_perf_table_[kVop3VMSadU8] = 4;
    quarter_device_perf_table_[kVop3VSADHiU8] = 4;
    quarter_device_perf_table_[kVop3VSADU16] = 4;
    quarter_device_perf_table_[kVop3VSadU32] = 4;
    quarter_device_perf_table_[kVop3VLerpU8] = 4;
    quarter_device_perf_table_[kVop1VCvtF32Ubyte0] = 4;
    quarter_device_perf_table_[kVop1VCvtF32Ubyte1] = 4;
    quarter_device_perf_table_[kVop1VCvtF32Ubyte2] = 4;
    quarter_device_perf_table_[kVop1VCvtF32Ubyte3] = 4;
    quarter_device_perf_table_[kVop1VCvtOffF32I4] = 4;
    quarter_device_perf_table_[kVop1VCvtF16F32] = 4;
    quarter_device_perf_table_[kVop2VCvtPkrtzF16F32] = 4;
    quarter_device_perf_table_[kVop1VCvtF32F16] = 4;
    quarter_device_perf_table_[kVop2VCvtPknormI16F32] = 4;
    quarter_device_perf_table_[kVop2VCvtPknormU16F32] = 4;
    quarter_device_perf_table_[kVop2VCvtPkU16U32] = 4;
    quarter_device_perf_table_[kVop2VCvtPkI16I32] = 4;
    quarter_device_perf_table_[kVop3VFmaF32] = 4;
    quarter_device_perf_table_[kVop2VMulU32U24] = 4;
    quarter_device_perf_table_[kVop2VMulHIU32U24] = 4;
    quarter_device_perf_table_[kVop3VMadU32U24] = 4;
    quarter_device_perf_table_[kVop3VMulI32I24] = 4;
    quarter_device_perf_table_[kVop2VMulHII32I24] = 4;
    quarter_device_perf_table_[kVop3VMadI32I24] = 4;
    quarter_device_perf_table_[kVop3VMulLitF32] = 4;
    quarter_device_perf_table_[kVop2VCndmaskB32] = 4;
    quarter_device_perf_table_[kVop3VCubeidF32] = 4;
    quarter_device_perf_table_[kVop3VCubescF32] = 4;
    quarter_device_perf_table_[kVop3VCubetcF32] = 4;
    quarter_device_perf_table_[kVop3VCubemaF32] = 4;
    quarter_device_perf_table_[kVop3VMaxF32] = 4;
    quarter_device_perf_table_[kVop3VMaxI32] = 4;
    quarter_device_perf_table_[kVop3VMaxU32] = 4;
    quarter_device_perf_table_[kVop3VMin3F32] = 4;
    quarter_device_perf_table_[kVop3VMin3I32] = 4;
    quarter_device_perf_table_[kVop3VMin3U32] = 4;
    quarter_device_perf_table_[kVop3VMed3F32] = 4;
    quarter_device_perf_table_[kVop3VMed3I32] = 4;
    quarter_device_perf_table_[kVop3VMed3U32] = 4;
    quarter_device_perf_table_[kVop3VCvtpkU8F32] = 4;
    quarter_device_perf_table_[kVop1VFRexpMantF32] = 4;
    quarter_device_perf_table_[kVop1VFRexpExpI32F32] = 4;
    quarter_device_perf_table_[kVop2VLDexpF32] = 4;
    quarter_device_perf_table_[kVopcVCmpeqF32] = 4;
    quarter_device_perf_table_[kVopcVCmpleF32] = 4;
    quarter_device_perf_table_[kVopcVCmpgtF32] = 4;
    quarter_device_perf_table_[kVopcVCmplgF32] = 4;
    quarter_device_perf_table_[kVopcVCmpgeF32] = 4;
    quarter_device_perf_table_[kVopcVCmpOF32] = 4;
    quarter_device_perf_table_[kVopcVCmpUF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNgeF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNlgF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNgtF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNleF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNeqF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNltF32] = 4;
    quarter_device_perf_table_[kVopcVCmpTruF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNgeF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNlgF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNgtF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNleF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNeqF32] = 4;
    quarter_device_perf_table_[kVopcVCmpNltF32] = 4;
    quarter_device_perf_table_[kVopcVCmpTruF32] = 4;
    quarter_device_perf_table_[kVopcVCmpFI32] = 4;
    quarter_device_perf_table_[kVopcVCmpLtI32] = 4;
    quarter_device_perf_table_[kVopcVCmpEqI32] = 4;
    quarter_device_perf_table_[kVopcVCmpLeI32] = 4;
    quarter_device_perf_table_[kVopcVCmpGtI32] = 4;
    quarter_device_perf_table_[kVopcVCmpNeI32] = 4;
    quarter_device_perf_table_[kVopcVCmpGeI32] = 4;
    quarter_device_perf_table_[kVopcVCmpTI32] = 4;
    quarter_device_perf_table_[kVopcVCmpxFI32] = 4;
    quarter_device_perf_table_[kVopcVCmpxLtI32] = 4;
    quarter_device_perf_table_[kVopcVCmpxEqI32] = 4;
    quarter_device_perf_table_[kVopcVCmpxLeI32] = 4;
    quarter_device_perf_table_[kVopcVCmpxGtI32] = 4;
    quarter_device_perf_table_[kVopcVCmpxNeI32] = 4;
    quarter_device_perf_table_[kVopcVCmpxGeI32] = 4;
    quarter_device_perf_table_[kVopcVCmpxTI32] = 4;
    quarter_device_perf_table_[kVopcVCmpFU32] = 4;
    quarter_device_perf_table_[kVopcVCmpLtU32] = 4;
    quarter_device_perf_table_[kVopcVCmpEqU32] = 4;
    quarter_device_perf_table_[kVopcVCmpLeU32] = 4;
    quarter_device_perf_table_[kVopcVCmpGtU32] = 4;
    quarter_device_perf_table_[kVopcVCmpNeU32] = 4;
    quarter_device_perf_table_[kVopcVCmpGeU32] = 4;
    quarter_device_perf_table_[kVopcVCmpTU32] = 4;
    quarter_device_perf_table_[kVopcVCmpxFU32] = 4;
    quarter_device_perf_table_[kVopcVCmpxLtU32] = 4;
    quarter_device_perf_table_[kVopcVCmpxEqU32] = 4;
    quarter_device_perf_table_[kVopcVCmpxGtU32] = 4;
    quarter_device_perf_table_[kVopcVCmpxNeU32] = 4;
    quarter_device_perf_table_[kVopcVCmpxGeU32] = 4;
    quarter_device_perf_table_[kVopcVCmpxTU32] = 4;
    quarter_device_perf_table_[kVopcVCmpClassF32] = 4;
    quarter_device_perf_table_[kVIVAddF16] = 4;
    quarter_device_perf_table_[kVIVMulF16] = 4;
    quarter_device_perf_table_[kVIVMadF16] = 4;
    quarter_device_perf_table_[kVIVFmaF16] = 4;
    quarter_device_perf_table_[kVIVDivScaleF16] = 4;
    quarter_device_perf_table_[kVIVDivFmasF16] = 4;
    quarter_device_perf_table_[kFra] = 4;
    quarter_device_perf_table_[kViVFractF16] = 4;
    quarter_device_perf_table_[kViVTruncF16] = 4;
    quarter_device_perf_table_[kViVCeilF16] = 4;
    quarter_device_perf_table_[kViVRndneF16] = 4;
    quarter_device_perf_table_[kViVFloorF16] = 4;
    quarter_device_perf_table_[kViVFrexpMantF16] = 4;
    quarter_device_perf_table_[kViVFrexpExpI16F16] = 4;
    quarter_device_perf_table_[kViVLdexpF16] = 4;
    quarter_device_perf_table_[kViVMaxF16] = 4;
    quarter_device_perf_table_[kViVMinF16] = 4;
    quarter_device_perf_table_[kViVinterpp1llF16] = 4;
    quarter_device_perf_table_[kViVinterpp1lvF16] = 4;
    quarter_device_perf_table_[kViVinterpp2F16] = 4;
    quarter_device_perf_table_[kViVAddU16] = 4;
    quarter_device_perf_table_[kViVSubI16] = 4;
    quarter_device_perf_table_[kViVSubU16] = 4;
    quarter_device_perf_table_[kViVMulI16] = 4;
    quarter_device_perf_table_[kViVMulU16] = 4;
    quarter_device_perf_table_[kViVMadI16] = 4;
    quarter_device_perf_table_[kViVMadU16] = 4;
    quarter_device_perf_table_[kViVMaxI16] = 4;
    quarter_device_perf_table_[kViVMinI16] = 4;
    quarter_device_perf_table_[kViVMaxU16] = 4;
    quarter_device_perf_table_[kViVMinU16] = 4;
    quarter_device_perf_table_[kViVAshrI16] = 4;
    quarter_device_perf_table_[kViVLshrB16] = 4;
    quarter_device_perf_table_[kViVLshlB16] = 4;
    quarter_device_perf_table_[kViVCvtI16F16] = 4;
    quarter_device_perf_table_[kViVCvtU16F16] = 4;
    quarter_device_perf_table_[kViVCvtF16I16] = 4;
    quarter_device_perf_table_[kViVCvtF16U16] = 4;
    quarter_device_perf_table_[kViVCvtPERMB32] = 4;
    quarter_device_perf_table_[kVop3VDivScaleF32] = 4;
    quarter_device_perf_table_[kVop3VDivFmasF32] = 4;
    quarter_device_perf_table_[kVop3VDivFixupF32] = 4;
    quarter_device_perf_table_[kVop1VFractF32] = 4;
    quarter_device_perf_table_[kVop2VMaxLegacyF32] = 4;
    quarter_device_perf_table_[kVop1VTruncF32] = 4;
    quarter_device_perf_table_[kVop2VMinLegacyF32] = 4;
    quarter_device_perf_table_[kVop2VMinF32] = 4;
    quarter_device_perf_table_[kVop2VMaxF32] = 4;
    quarter_device_perf_table_[kVop2VCeilF32] = 4;
    quarter_device_perf_table_[kVop2VRndneF32] = 4;
    quarter_device_perf_table_[kVop2VFloorF32] = 4;
    quarter_device_perf_table_[kVop2VMinI32] = 4;
    quarter_device_perf_table_[kVop2VMaxU32] = 4;
    quarter_device_perf_table_[kVop2VMinU32] = 4;
    quarter_device_perf_table_[kVop2VASHRI32] = 4;

    // Half rate.
    quarter_device_perf_table_[kVop1VLogF32] = 8;
    quarter_device_perf_table_[kVOP1VLogClampF32] = 8;
    quarter_device_perf_table_[kVop1VCvtF32F64] = 8;
    quarter_device_perf_table_[kVop1VCvtI32F64] = 8;
    quarter_device_perf_table_[kVop1VCvtF64I32] = 8;
    quarter_device_perf_table_[kVop1VCvtU32F64] = 8;
    quarter_device_perf_table_[kVop1VCvtF64U32] = 8;
    quarter_device_perf_table_[kVop3VMinF64] = 8;
    quarter_device_perf_table_[kVop3VMaxF64] = 8;
    quarter_device_perf_table_[kVop3VLdexPF64] = 8;
    quarter_device_perf_table_[kVop3VFrexPMantF64] = 8;
    quarter_device_perf_table_[kVop3VFrexPExpI32F64] = 8;
    quarter_device_perf_table_[kVop1VFracTF64] = 8;
    quarter_device_perf_table_[kVop1VTrunCF64] = 8;
    quarter_device_perf_table_[kVop1VCeilF64] = 8;
    quarter_device_perf_table_[kVop1VRndnEF64] = 8;
    quarter_device_perf_table_[kVop1VFlooRF64] = 8;
    quarter_device_perf_table_[kVop3VAshrI64] = 8;
    quarter_device_perf_table_[kVop3VDivScaleF64] = 8;
    quarter_device_perf_table_[kVop3VDivFixupF64] = 8;
    quarter_device_perf_table_[kVopcVCmpFF64] = 8;
    quarter_device_perf_table_[kVopcVCmpLtF64] = 8;
    quarter_device_perf_table_[kVopcVCmpEqF64] = 8;
    quarter_device_perf_table_[kVopcVCmpLeF64] = 8;
    quarter_device_perf_table_[kVopcVCmpGtF64] = 8;
    quarter_device_perf_table_[kVopcVCmpLgF64] = 8;
    quarter_device_perf_table_[kVopcVCmpGeF64] = 8;
    quarter_device_perf_table_[kVopcVCmpOF64] = 8;
    quarter_device_perf_table_[kVopcVCmpUF64] = 8;
    quarter_device_perf_table_[kVopcVCmpNgeF64] = 8;
    quarter_device_perf_table_[kVopcVCmpNlgF64] = 8;
    quarter_device_perf_table_[kVopcVCmpNgtF64] = 8;
    quarter_device_perf_table_[kVopcVCmpNleF64] = 8;
    quarter_device_perf_table_[kVopcVCmpNeqF64] = 8;
    quarter_device_perf_table_[kVopcVCmpNltF64] = 8;
    quarter_device_perf_table_[kVopcVCmpTruF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxFF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxLtF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxEqF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxLeF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxGtF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxLgF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxGeF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxOF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxUF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxNgeF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxNlgF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxNgtF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxNleF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxNeqF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxNltF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxTruF64] = 8;
    quarter_device_perf_table_[kVopcVCmpFI64] = 8;
    quarter_device_perf_table_[kVopcVCmpLtI64] = 8;
    quarter_device_perf_table_[kVopcVCmpEqI64] = 8;
    quarter_device_perf_table_[kVopcVCmpLeI64] = 8;
    quarter_device_perf_table_[kVopcVCmpGtI64] = 8;
    quarter_device_perf_table_[kVopcVCmpLgI64] = 8;
    quarter_device_perf_table_[kVopcVCmpGeI64] = 8;
    quarter_device_perf_table_[kVopcVCmpTI64] = 8;
    quarter_device_perf_table_[kVopcVCmpxFI64] = 8;
    quarter_device_perf_table_[kVopcVCmpxLtI64] = 8;
    quarter_device_perf_table_[kVopcVCmpxEqI64] = 8;
    quarter_device_perf_table_[kVopcVCmpxLeI64] = 8;
    quarter_device_perf_table_[kVopcVCmpxGtI64] = 8;
    quarter_device_perf_table_[kVopcVCmpxLgI64] = 8;
    quarter_device_perf_table_[kVopcVCmpxGeI64] = 8;
    quarter_device_perf_table_[kVopcVCmpxTI64] = 8;
    quarter_device_perf_table_[kVopcVCmpFU64] = 8;
    quarter_device_perf_table_[kVopcVCmpLtU64] = 8;
    quarter_device_perf_table_[kVopcVCmpEqU64] = 8;
    quarter_device_perf_table_[kVopcVCmpLeU64] = 8;
    quarter_device_perf_table_[kVopcVCmpGtU64] = 8;
    quarter_device_perf_table_[kVopcVCmpLgU64] = 8;
    quarter_device_perf_table_[kVopcVCmpGeU64] = 8;
    quarter_device_perf_table_[kVopcVCmpTU64] = 8;
    quarter_device_perf_table_[kVopcVCmpxFU64] = 8;
    quarter_device_perf_table_[kVopcVCmpxLtU64] = 8;
    quarter_device_perf_table_[kVopcVCmpxEqU64] = 8;
    quarter_device_perf_table_[kVopcVCmpxLeU64] = 8;
    quarter_device_perf_table_[kVopcVCmpxGtU64] = 8;
    quarter_device_perf_table_[kVopcVCmpxLgU64] = 8;
    quarter_device_perf_table_[kVopcVCmpxGeU64] = 8;
    quarter_device_perf_table_[kVopcVCmpxTU64] = 8;
    quarter_device_perf_table_[kVopcVCmpClassF64] = 8;
    quarter_device_perf_table_[kVopcVCmpxClassF64] = 8;

    // Quarter rate.
    quarter_device_perf_table_[kVOP1VSqrtF32] = 16;
    quarter_device_perf_table_[kVOP3VMulLoI32] = 16;
    quarter_device_perf_table_[kVOP3VMulHiI32] = 16;
    quarter_device_perf_table_[kVOP3VMulLoU32] = 16;
    quarter_device_perf_table_[kVOP3VMulHiU32] = 16;
    quarter_device_perf_table_[kVOP3VMadU64U32] = 16;
    quarter_device_perf_table_[kVOP3VMadI64I32] = 16;
    quarter_device_perf_table_[kVOP1VExpF32] = 16;
    quarter_device_perf_table_[kVOP1VLogClampF32] = 16;
    quarter_device_perf_table_[kVOP1VRcpClampF32] = 16;
    quarter_device_perf_table_[kVOP1VRcpLegacyF32] = 16;
    quarter_device_perf_table_[kVOP1VRcpF32] = 16;
    quarter_device_perf_table_[kVOP1VRcpIFlagF32] = 16;
    quarter_device_perf_table_[kVOP1VRsqClampF32] = 16;
    quarter_device_perf_table_[kVOP1VRsqLegacyF32] = 16;
    quarter_device_perf_table_[kVOP1VRsqF32] = 16;
    quarter_device_perf_table_[kVOP1VSqrtF32] = 16;
    quarter_device_perf_table_[kVOP1VSinF32] = 16;
    quarter_device_perf_table_[kVOP1VCosF32] = 16;
    quarter_device_perf_table_[kVOP1VRcpF64] = 16;
    quarter_device_perf_table_[kVOP1VRcpClampF64] = 16;
    quarter_device_perf_table_[kVOP1VRsqF64] = 16;
    quarter_device_perf_table_[kVOP1VRsqClampF64] = 16;
    quarter_device_perf_table_[kVOP3VTrigPreopF64] = 16;
    quarter_device_perf_table_[kVOP3VMulF64] = 16;
    quarter_device_perf_table_[kVOP3VFmaF64] = 16;
    quarter_device_perf_table_[kVOP3VDivFmasF64] = 16;
    quarter_device_perf_table_[kVOP3VMqsadPkU16U8] = 16;
    quarter_device_perf_table_[kVOP3VMqsadU32U8] = 16;
    quarter_device_perf_table_[kViVRcpF16] = 16;
    quarter_device_perf_table_[kViVSqrtF16] = 16;
    quarter_device_perf_table_[kViVRsqF16] = 16;
    quarter_device_perf_table_[kViVExpF16] = 16;
    quarter_device_perf_table_[kViVLogF16] = 16;
    quarter_device_perf_table_[kViVSinF16] = 16;
    quarter_device_perf_table_[kViVCosF16] = 16;
    quarter_device_perf_table_[kViQSadPkU16U8] = 16;
    quarter_device_perf_table_[kVOP3VMulF64] = 16;
    quarter_device_perf_table_[kVOP3VFmaF64] = 16;
    quarter_device_perf_table_[kVop1VSqrtF64] = 16;
    quarter_device_perf_table_[kVOP1VRsqClampF64] = 16;
    quarter_device_perf_table_[kVOP1VRsqF64] = 16;
}

std::string Instruction::GetFunctionalUnitAsString(InstructionCategory category)
{
    static const char* kFunctionalUnitAtomics = "Atomics";

    std::string ret;

    switch (category)
    {
        case kScalarAlu:
            ret = FUNC_UNIT_SALU;
            break;

        case kScalarMemoryRead:
        case kScalarMemoryWrite:
            ret = FUNC_UNIT_SMEM;
            break;

        case kVectorMemoryRead:
        case kVectorMemoryWrite:
            ret = FUNC_UNIT_VMEM;
            break;

        case kVectorAlu:
            ret = FUNC_UNIT_VALU;
            break;

        case kLds:
            ret = FUNC_UNIT_LDS;
            break;

        case kGds:
        case kExport:
            ret = FUNC_UNIT_GDS_EXPORT;
            break;

        case kAtomics:
            ret = kFunctionalUnitAtomics;
            break;

        case kInternal:
            // We will still name it "Flow Control"
            // so that it will be meaningful for the end user.
            ret = FUNC_UNIT_INTERNAL_FLOW;
            break;

        case kBranch:
            ret = FUNC_UNIT_BRANCH;
            break;

        default:
            ret = FUNC_UNIT_UNKNOWN;

            // We shouldn't get here.
            GT_ASSERT(false);
            break;
    }

    return ret;
}

Instruction::Instruction(const std::string& labelString)
{
    // Setup the performance tables.
    SetUpPerfTables();

    pointing_label_string_ = labelString;

    size_t strLen = pointing_label_string_.size();

    // Remove terminating carriage return character from the label if it exists
    if (!pointing_label_string_.empty() && pointing_label_string_[strLen - 1] == '\r')
    {
        pointing_label_string_.erase(strLen - 1);
    }
}

Instruction::Instruction(unsigned int instruction_width, InstructionCategory instruction_format_kind, InstructionSet instruction_format, int label /*= kNoLabel*/, int goto_label /*= kNoLabel*/) :
    m_instructionWidth(instruction_width), Instruction_category_(instruction_format_kind), instruction_format_(instruction_format), label_(label), goto_label_(goto_label), line_number_(0), hw_gen_(GDT_HW_GENERATION_SOUTHERNISLAND)
{
    // Setup the performance tables.
    SetUpPerfTables();
}

void Instruction::GetCsvString(const std::string& device_name, bool should_add_src_line_info, std::string& csv_string)const
{
    std::stringstream output_stream;

    if (GetLabel() == kNoLabel)
    {
        std::string full_offset = GetInstructionOffset().c_str();
        size_t len = full_offset.size() >= 6 ? full_offset.size() - 6 : 0;

        output_stream << "0x" << full_offset.substr(len, 6) << COMMA_SEPARATOR;

        // Add the source line info.
        if (should_add_src_line_info)
        {
            auto  srcLineInfo = GetSrcLineInfo();
            output_stream << srcLineInfo.first << COMMA_SEPARATOR;
        }

        bool is_branch = (GetInstructionCategory() == InstructionCategory::kBranch);
        output_stream << GetInstructionOpCode() << COMMA_SEPARATOR;
        output_stream << DOUBLE_QUOTES << GetInstructionParameters() << DOUBLE_QUOTES << COMMA_SEPARATOR;

        output_stream << Instruction::GetFunctionalUnitAsString(GetInstructionCategory()) << COMMA_SEPARATOR;

        // Get the number of cycle that this instruction costs.
        int cycle_count = GetInstructionClockCount(device_name);

        if (cycle_count > 0)
        {
            output_stream << cycle_count;
        }
        else
        {
            if (is_branch)
            {
                output_stream << BRANCH_CYCLES;
            }
            else
            {
                output_stream << NA_VALUE;
            }
        }

        output_stream << COMMA_SEPARATOR;
        output_stream << GetInstructionBinaryRep() << COMMA_SEPARATOR;

    }
    else
    {
        output_stream << GetPointingLabelString();
    }

    output_stream << NEWLINE_SEPARATOR;
    csv_string = output_stream.str();
}
