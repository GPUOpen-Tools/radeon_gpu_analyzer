//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for class representing MIMG Instruction in shader isa.
//=============================================================================

#ifndef __MIMGINSTRUCTION_H
#define __MIMGINSTRUCTION_H
#include "be_instruction.h"

// Image memory buffer operations. Two words.
// Opcode :
//        DMASK  [11:8]
//        UNORM  [12:12]
//        GLC    [13:13]
//        DA     [14:14]
//        R128   [15:15]
//        TFE    [16:16]
//        LWE    [17:17]
//        OP     [24:18]
//        SLC    [25:25]
//        VADDR  [39:32]
//        VDATA  [47:40]
//        SRSRC  [52:48]
//        SSAMP  [57:53]

class MIMGInstruction : public Instruction
{
public:

    typedef short DMASK;
    typedef char UNORM;
    typedef char GLC;
    typedef char DA;
    typedef char R128;
    typedef char TFE;
    typedef char LWE;
    typedef char SLC;
    typedef char VADDR;
    typedef char VDATA;
    typedef char SRSRC;
    typedef char SSAMP;

private:
    // Enable mask for image read/write data components. bit0 = red, 1 = green,
    // 2 = blue, 3 = alpha. At least one bit must be on. Data is assumed to be packed
    // into consecutive VGPRs.
    DMASK dmask_;

    // IWhen set to 1, forces the address to be un-normalized, regardless of T#. Must be
    // set to 1 for image stores and atomics
    UNORM unorm_;


    // If set, operation is globally coherent.
    GLC glc_;

    // Declare an Array.
    // 1 Kernel has declared this resource to be an array of texture maps.
    // 0 Kernel has declared this resource to be a single texture map.
    DA da_;

    // Texture resource size: 1 = 128b, 0 = 256b.
    R128 r128_;

    // Texture Fail Enable (for partially resident textures).
    TFE tfe_;

    // LOD Warning Enable (for partially resident textures).
    LWE lwe_;

    // System Level Coherent.
    SLC slc_;

    // VGPR address source. Can carry an offset or an index.
    VADDR vaddr_;

    // Vector GPR to read/write result to.
    VDATA vdata_;

    // Scalar GPR that specifies the resource constant, in units of four SGPRs.
    SRSRC srsrc_;

    // Scalar GPR that specifies the sampler constant, in units of four SGPRs.
    SSAMP ssamp_;

    // MTBUF Instruction Width in bits
    static const unsigned int MIMGInstructionWidth = 64;
public:
    MIMGInstruction(DMASK dmask, UNORM unorm, GLC glc, DA da, R128 r128, TFE tfe, LWE lwe, SLC slc, VADDR vaddr, VDATA vdata, SRSRC srsrc,
                    SSAMP ssamp, Instruction::InstructionCategory inst, int label_, int iGotoLabel): Instruction(MIMGInstructionWidth, inst, kInstructionSetMimg, label_, iGotoLabel),
        dmask_(dmask), unorm_(unorm), glc_(glc), da_(da), r128_(r128), tfe_(tfe), lwe_(lwe),
        slc_(slc), vaddr_(vaddr), vdata_(vdata), srsrc_(srsrc), ssamp_(ssamp) {}

    virtual ~MIMGInstruction() = default;

    // Get the DMASK  [11:8].
    DMASK GetDmask() const { return dmask_; }

    // Get the UNORM  [12:12].
    UNORM GetUnorm() const { return unorm_; }

    // Get the GLC    [14:14].
    GLC GetGlc() const { return glc_; }

    // Get the  DA     [14:14].
    DA GetDa() const { return da_; }

    // Get the R128   [15:15].
    R128 GetR128() const { return r128_; }

    // Get the TFE    [16:16].
    TFE GetTfe() const { return tfe_; }

    // Get the LWE    [17:17].
    LWE GetLwe() const { return lwe_; }

    // Get the SLC    [25:25].
    SLC GetSlc() const { return slc_; }

    // Get the VADDR  [39:32].
    VADDR GetVaddr() const { return vaddr_; }

    // Get the VDATA  [47:40].
    VDATA GetVdata() const { return vdata_; }

    // Get the SRSRC  [52:48].
    SRSRC GetSrsrc() const { return srsrc_; }

    // Get the SSAMP  [57:53].
    SSAMP GetSsamp() const { return ssamp_; }
};

class SIMIMGInstruction : public MIMGInstruction
{
public:
    // Selector for the MIMG Instruction
    enum OP
    {
        //  Image memory load with format conversion specified in T#. No
        // sampler.
        kImageLoad,
        //  Image memory load with user-supplied mip level. No sampler.
        kImageLoadMip,
        //  Image memory load with no format conversion. No sampler.
        kImageLoadPck,
        //  Image memory load with with no format conversion
        // and sign extension. No sampler.
        kImageLoadPck_SGN,
        //  Image memory load with user-supplied mip level, no
        // format conversion. No sampler.
        kImageLoadMipPck,
        //  Image memory load with user-supplied mip
        // level, no format conversion and with sign extension. No sampler.
        kImageLoadMipPckSgn,
        //  Image memory store with format conversion specified in T#.
        // No sampler.
        kImageStore,
        //  Image memory store with format conversion specified in
        // T# to user specified mip level. No sampler.
        kImageStoreMip,
        //  Image memory store of packed data without format conversion.
        // No sampler.
        kImageStorePck,
        //  Image memory store of packed data without format
        // conversion to user-supplied mip level. No sampler.
        kImageStoreMipPck,
        //  return resource info. No sampler.
        kImageGetResinfo,
        //  dst=src, returns previous value if glc==1.
        kImageAtomic_SWAP,
        //  dst = (dst==cmp) ? src : dst. Returns previous
        // value if glc==1.
        kImageAtomic_CMPSWAP,
        //  dst += src. Returns previous value if glc==1.
        kImageAtomic_ADD,
        //  dst -= src. Returns previous value if glc==1.
        kImageAtomic_SUB,
        //  dst = src-dst. Returns previous value if glc==1.
        kImageAtomic_RSUB,
        //  dst = (src < dst) ? src : dst (signed). Returns previous
        // value if glc==1.
        kImageAtomic_SMIN,
        //  dst = (src < dst) ? src : dst (unsigned). Returns previous
        // value if glc==1.
        kImageAtomicUmin,
        //  dst = (src > dst) ? src : dst (signed). Returns previous
        // value if glc==1.
        kImageAtomicSmax,
        //  dst = (src > dst) ? src : dst (unsigned). Returns previous
        // value if glc==1.
        kImageAtomicUmax,
        //  dst &= src. Returns previous value if glc==1.
        kImageAtomicAnd,
        //  dst |= src. Returns previous value if glc==1.
        kImageAtomicOr,
        //  dst ^= src. Returns previous value if glc==1.
        kImageAtomicXor,
        //  dst = (dst >= src) ? 0 : dst+1. Returns previous value if
        // glc==1.
        kImageAtomicInc,
        //  dst = ((dst==0 || (dst > src)) ? src : dst-1. Returns previous
        // value if glc==1.
        kImageAtomicDec,
        //  dst = (dst == cmp) ? src : dst, returns previous
        // value of dst if glc==1 - double and float atomic compare swap. Obeys floating
        // point compare rules for special values.
        kImageAtomicFcmpswap,
        //  dst = (src < dst) ? src : dst, returns previous value of
        // dst if glc==1 - double and float atomic min (handles NaN/INF/denorm).
        kImageAtomicFmin,
        //  dst = (src > dst) ? src : dst, returns previous value of
        // dst if glc==1 - double and float atomic min (handles NaN/INF/denorm).
        kImageAtomicFmax,
        //  sample texture map.
        kImageSample,
        //  sample texture map, with LOD clamp specified in
        // shader.
        kImageSampleCl ,
        //  sample texture map, with user derivatives.
        kImageSampleD,
        //  sample texture map, with LOD clamp specified in
        // shader, with user derivatives.
        kImageSampleDCl,
        //  sample texture map, with user LOD.
        kImageSampleL,
        //  sample texture map, with lod bias.
        kImageSampleB,
        //  sample texture map, with LOD clamp specified in
        // shader, with lod bias.
        kImageSampleBCl,
        //  sample texture map, from level 0.
        kImageSampleLz,
        //  sample texture map, with PCF.
        kImageSampleC,
        //  SAMPLE_C, with LOD clamp specified in shader.
        kImageSampleCCl,
        //  SAMPLE_C, with user derivatives.
        kImageSampleCD,
        //  SAMPLE_C, with LOD clamp specified in shader,
        // with user derivatives.
        kImageSampleCDCl,
        //  SAMPLE_C, with user LOD.
        kImageSampleCL,
        //  SAMPLE_C, with lod bias.
        kImageSampleCB,
        //  SAMPLE_C, with LOD clamp specified in shader,
        // with lod bias.
        kImageSampleCBCl,
        //  SAMPLE_C, from level 0.
        kImageSampleCLz,
        //  sample texture map, with user offsets.
        kImageSampleO,
        //  SAMPLE_O with LOD clamp specified in shader.
        kImageSampleClO,
        //  SAMPLE_O, with user derivatives.
        kImageSampleDO,
        //  SAMPLE_O, with LOD clamp specified in shader,
        // with user derivatives.
        kImageSampleDClO,
        //  SAMPLE_O, with user LOD.
        kImageSampleLO,
        //  SAMPLE_O, with lod bias.
        kImageSampleBO,
        //  SAMPLE_O, with LOD clamp specified in shader,
        // with lod bias.
        kImageSampleBClO,
        //  SAMPLE_O, from level 0.
        kImageSampleLzO,
        //  SAMPLE_C with user specified offsets.
        kImageSampleCO,
        //  SAMPLE_C_O, with LOD clamp specified in
        // shader.
        kImageSampleCClO,
        //  SAMPLE_C_O, with user derivatives.
        kImageSampleCDO,
        //  SAMPLE_C_O, with LOD clamp specified in
        // shader, with user derivatives.
        kImageSampleCDClO,
        //  SAMPLE_C_O, with user LOD.
        kImageSampleCLO,
        //  SAMPLE_C_O, with lod bias.
        kImageSampleCBO,
        //  SAMPLE_C_O, with LOD clamp specified in
        // shader, with lod bias.
        kImageSample_CBClO,
        //  SAMPLE_C_O, from level 0.
        kImageSample_CLzO,
        //  gather 4 single component elements (2x2).
        kImageGather4,
        //  gather 4 single component elements (2x2) with user
        // LOD clamp.
        kImageGather4Cl,
        //  GATHER4 single component elements (2x2) with user
        // LOD.
        kImageGather4L,
        //  GATHER4 single component elements (2x2) with user
        // bias.
        kImageGather4B,
        //  GATHER4 single component elements (2x2) with user
        // bias and clamp.
        kImageGather4BCl,
        //  GATHER4 single component elements (2x2) at level 0.
        kImageGather4Lz,
        //  GATHER4 single component elements (2x2) with PCF.
        kImageGather4C,
        //  GATHER4 single component elements (2x2) with user
        // LOD clamp and PCF.
        kImageGather4CCl,
        //  GATHER4 single component elements (2x2) with user
        // LOD and PCF.
        kImageGather4CL,
        //  GATHER4 single component elements (2x2) with user
        // bias and PCF.
        kImageGather4CB,
        //  GATHER4 single component elements (2x2) with
        // user bias, clamp and PCF.
        kImageGather4CBCl,
        //  GATHER4 single component elements (2x2) at level 0,
        // with PCF.
        kImageGather4CLz,
        //  GATHER4, with user offsets.
        kImageGather4O,
        //  GATHER4_CL, with user offsets.
        kImageGather4ClO,
        //  GATHER4_L, with user offsets.
        kImageGather4LO,
        //  GATHER4_B, with user offsets.
        kImageGather4BO,
        //  GATHER4_B_CL, with user offsets.
        kImageGather4BClO,
        //  GATHER4_LZ, with user offsets.
        kImageGather4LzO,
        //  GATHER4_C, with user offsets.
        kImageGather4CO,
        //  GATHER4_C_CL, with user offsets.
        kImageGather4CClO,
        //  GATHER4_C_L, with user offsets.
        kImageGather4CLO,
        //  GATHER4_B, with user offsets.
        kImageGather4CBO,
        //  Gather4_B_CL, with user offsets.
        kImageGather4CBClO,
        //  Gather4_C_LZ, with user offsets.
        kImageGather4CLzO,
        //  Return calculated LOD.
        kImageGetLod,
        //  sample texture map, with user derivatives (LOD per
        // quad)
        kImageSampleCd,
        //  sample texture map, with LOD clamp specified in
        // shader, with user derivatives (LOD per quad).
        kImageSampleCdCl,
        //  SAMPLE_C, with user derivatives (LOD per quad).
        kImageSampleCCd,
        //  SAMPLE_C, with LOD clamp specified in shader,
        // with user derivatives (LOD per quad).
        kImageSampleCCdCl,
        //  SAMPLE_O, with user derivatives (LOD per quad).
        kImageSampleCdO,
        //  SAMPLE_O, with LOD clamp specified in shader,
        // with user derivatives (LOD per quad).
        kImageSampleCdClO,
        //  SAMPLE_C_O, with user derivatives (LOD per
        // quad).
        kImageSampleCCdO,
        //  SAMPLE_C_O, with LOD clamp specified in
        // shader, with user derivatives (LOD per quad).
        // All other values are reserved.
        kImageSampleCCdClO,
        // Reserved
        kImageReserved
    };

    // Get the     OP [24:18].
    OP GetOp() const { return op_; }

    // ctor
    SIMIMGInstruction(DMASK dmask, UNORM unorm, GLC glc, DA da, R128 r128, TFE tfe, LWE lwe, OP op, SLC slc, VADDR vaddr, VDATA vdata, SRSRC srsrc,
                      SSAMP ssamp, Instruction::InstructionCategory inst, int label, int goto_label):
        MIMGInstruction(dmask, unorm, glc, da, r128, tfe, lwe, slc, vaddr, vdata, srsrc, ssamp, inst, label, goto_label), op_(op) {}

private:
    // MTBUF operation.
    OP op_;

};

class VIMIMGInstruction : public MIMGInstruction
{
public:
    // Selector for the MIMG Instruction
    enum OP
    {
        kImageLoad = 0,
        kImageLoadMip = 1,
        kImageLoadPck = 2,
        kImageLoadPckSgn = 3,
        kImageLoadMipPck = 4,
        kImageLoadMipPckSgn = 5,
        kImageStore = 8,
        kImageStoreMip = 9,
        kImageStorePck = 10,
        kImageStoreMipPck = 11,
        kImageGetResinfo = 14,
        kImageAtomicSwap = 16,
        kImageAtomicCmpswap = 17,
        kImageAtomicAdd = 18,
        kImageAtomicSub = 19,
        kImageAtomicSmin = 20,
        kImageAtomicUmin = 21,
        kImageAtomicSmax = 22,
        kImageAtomicUmax = 23,
        kImageAtomicAnd = 24,
        kImageAtomicOr = 25,
        kImageAtomicXor = 26,
        kImageAtomicInc = 27,
        kImageAtomicDec = 28,
        kImageSample = 32,
        kImageSampleCl = 33,
        kImageSampleD = 34,
        kImageSampleDCl = 35,
        kImageSampleL = 36,
        kImageSampleB = 37,
        kImageSampleBCl = 38,
        kImageSampleLz = 39,
        kImageSampleC = 40,
        kImageSampleCCl = 41,
        kImageSampleCD = 42,
        kImageSampleCDCl = 43,
        kImageSampleCL = 44,
        kImageSampleCB = 45,
        kImageSampleCBCl = 46,
        kImageSampleCLz = 47,
        kImageSampleO = 48,
        kImageSampleClO = 49,
        kImageSampleDO = 50,
        kImageSampleDClO = 51,
        kImageSampleLO = 52,
        kImageSampleBO = 53,
        kImageSampleBClO = 54,
        kImageSampleLzO = 55,
        kImageSampleCO = 56,
        kImageSampleCClO = 57,
        kImageSampleCDO = 58,
        kImageSampleCDClO = 59,
        kImageSampleCLO = 60,
        kImageSampleCBO = 61,
        kImageSampleCBClO = 62,
        kImageSampleCLzO = 63,
        kImageGather4 = 64,
        kImageGather4Cl = 65,
        kImageGather4L = 68,
        kImageGather4B = 69,
        kImageGather4BCl = 70,
        kImageGather4Lz = 71,
        kImageGather4C = 72,
        kImageGather4CCl = 73,
        kImageGather4CL = 76,
        kImageGather4CB = 77,
        kImageGather4CBCl = 78,
        kImageGather4CLz = 79,
        kImageGather4O = 80,
        kImageGather4ClO = 81,
        kImageGather4LO = 84,
        kImageGather4BO = 85,
        kImageGather4BClO = 86,
        kImageGather4LzO = 87,
        kImageGather4CO = 88,
        kImageGather4CClO = 89,
        kImageGather4CLO = 92,
        kImageGather4CBO = 93,
        kImageGather4CBClO = 94,
        kImageGather4CLzO = 95,
        kImageGetLod = 96,
        kImageSampleCd = 104,
        kImageSampleCdCl = 105,
        kImageSampleCCd = 106,
        kImageSampleCCdCl = 107,
        kImageSampleCdO = 108,
        kImageSampleCdClO = 109,
        kImageSampleCCdO = 110,
        kImageSampleCCdClO = 111,
        kImageIlegal = 112,
    };

    // Get the     OP [24:18]
    OP GetOp() const { return op_; }

    // ctor
    VIMIMGInstruction(DMASK dmask, UNORM unorm, GLC glc, DA da, R128 r128, TFE tfe, LWE lwe, OP op, SLC slc, VADDR vaddr, VDATA vdata, SRSRC srsrc,
                      SSAMP ssamp, Instruction::InstructionCategory inst, int label, int goto_label): MIMGInstruction(dmask, unorm, glc, da,
                          r128, tfe, lwe, slc, vaddr, vdata, srsrc, ssamp, inst, label, goto_label), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

private:
    // MTBUF operation.
    OP op_;

};
#endif //__MIMGINSTRUCTION_H

