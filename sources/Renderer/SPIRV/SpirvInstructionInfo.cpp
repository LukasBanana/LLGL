/*
 * SpirvInstructionInfo.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "SpirvInstructionInfo.h"


namespace LLGL
{


static bool HasSpirvInstructionTypeId(const spv::Op opCode)
{
    switch (opCode)
    {
        case spv::OpUndef:
        case spv::OpSizeOf:
        case spv::OpMemberName:
        case spv::OpExtInst:
        case spv::OpTypeForwardPointer:
        case spv::OpConstantTrue:
        case spv::OpConstantFalse:
        case spv::OpConstant:
        case spv::OpConstantComposite:
        case spv::OpConstantSampler:
        case spv::OpConstantNull:
        case spv::OpSpecConstantTrue:
        case spv::OpSpecConstantFalse:
        case spv::OpSpecConstant:
        case spv::OpSpecConstantComposite:
        case spv::OpSpecConstantOp:
        case spv::OpVariable:
        case spv::OpImageTexelPointer:
        case spv::OpLoad:
        case spv::OpAccessChain:
        case spv::OpInBoundsAccessChain:
        case spv::OpPtrAccessChain:
        case spv::OpArrayLength:
        case spv::OpGenericPtrMemSemantics:
        case spv::OpInBoundsPtrAccessChain:
        case spv::OpFunction:
        case spv::OpFunctionParameter:
        case spv::OpFunctionCall:
        case spv::OpSampledImage:
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageFetch:
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageRead:
        case spv::OpImage:
        case spv::OpImageQueryFormat:
        case spv::OpImageQueryOrder:
        case spv::OpImageQuerySizeLod:
        case spv::OpImageQuerySize:
        case spv::OpImageQueryLod:
        case spv::OpImageQueryLevels:
        case spv::OpImageQuerySamples:
        case spv::OpImageSparseSampleImplicitLod:
        case spv::OpImageSparseSampleExplicitLod:
        case spv::OpImageSparseSampleDrefImplicitLod:
        case spv::OpImageSparseSampleDrefExplicitLod:
        case spv::OpImageSparseSampleProjImplicitLod:
        case spv::OpImageSparseSampleProjExplicitLod:
        case spv::OpImageSparseSampleProjDrefImplicitLod:
        case spv::OpImageSparseSampleProjDrefExplicitLod:
        case spv::OpImageSparseFetch:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
        case spv::OpImageSparseTexelsResident:
        case spv::OpImageSparseRead:
        case spv::OpConvertFToU:
        case spv::OpConvertFToS:
        case spv::OpConvertSToF:
        case spv::OpConvertUToF:
        case spv::OpUConvert:
        case spv::OpSConvert:
        case spv::OpFConvert:
        case spv::OpQuantizeToF16:
        case spv::OpConvertPtrToU:
        case spv::OpSatConvertSToU:
        case spv::OpSatConvertUToS:
        case spv::OpConvertUToPtr:
        case spv::OpPtrCastToGeneric:
        case spv::OpGenericCastToPtr:
        case spv::OpGenericCastToPtrExplicit:
        case spv::OpBitcast:
        case spv::OpVectorExtractDynamic:
        case spv::OpVectorInsertDynamic:
        case spv::OpVectorShuffle:
        case spv::OpCompositeConstruct:
        case spv::OpCompositeExtract:
        case spv::OpCompositeInsert:
        case spv::OpCopyObject:
        case spv::OpTranspose:
        case spv::OpSNegate:
        case spv::OpFNegate:
        case spv::OpIAdd:
        case spv::OpFAdd:
        case spv::OpISub:
        case spv::OpFSub:
        case spv::OpIMul:
        case spv::OpFMul:
        case spv::OpUDiv:
        case spv::OpSDiv:
        case spv::OpFDiv:
        case spv::OpUMod:
        case spv::OpSRem:
        case spv::OpSMod:
        case spv::OpFRem:
        case spv::OpFMod:
        case spv::OpVectorTimesScalar:
        case spv::OpMatrixTimesScalar:
        case spv::OpVectorTimesMatrix:
        case spv::OpMatrixTimesVector:
        case spv::OpMatrixTimesMatrix:
        case spv::OpOuterProduct:
        case spv::OpDot:
        case spv::OpIAddCarry:
        case spv::OpISubBorrow:
        case spv::OpUMulExtended:
        case spv::OpSMulExtended:
        case spv::OpShiftRightLogical:
        case spv::OpShiftRightArithmetic:
        case spv::OpShiftLeftLogical:
        case spv::OpBitwiseOr:
        case spv::OpBitwiseXor:
        case spv::OpBitwiseAnd:
        case spv::OpNot:
        case spv::OpBitFieldInsert:
        case spv::OpBitFieldSExtract:
        case spv::OpBitFieldUExtract:
        case spv::OpBitReverse:
        case spv::OpBitCount:
        case spv::OpAny:
        case spv::OpAll:
        case spv::OpIsNan:
        case spv::OpIsInf:
        case spv::OpIsFinite:
        case spv::OpIsNormal:
        case spv::OpSignBitSet:
        case spv::OpLessOrGreater:
        case spv::OpOrdered:
        case spv::OpUnordered:
        case spv::OpLogicalEqual:
        case spv::OpLogicalNotEqual:
        case spv::OpLogicalOr:
        case spv::OpLogicalAnd:
        case spv::OpLogicalNot:
        case spv::OpSelect:
        case spv::OpIEqual:
        case spv::OpINotEqual:
        case spv::OpUGreaterThan:
        case spv::OpSGreaterThan:
        case spv::OpUGreaterThanEqual:
        case spv::OpSGreaterThanEqual:
        case spv::OpULessThan:
        case spv::OpSLessThan:
        case spv::OpULessThanEqual:
        case spv::OpSLessThanEqual:
        case spv::OpFOrdEqual:
        case spv::OpFUnordEqual:
        case spv::OpFOrdNotEqual:
        case spv::OpFUnordNotEqual:
        case spv::OpFOrdLessThan:
        case spv::OpFUnordLessThan:
        case spv::OpFOrdGreaterThan:
        case spv::OpFUnordGreaterThan:
        case spv::OpFOrdLessThanEqual:
        case spv::OpFUnordLessThanEqual:
        case spv::OpFOrdGreaterThanEqual:
        case spv::OpFUnordGreaterThanEqual:
        case spv::OpDPdx:
        case spv::OpDPdy:
        case spv::OpFwidth:
        case spv::OpDPdxFine:
        case spv::OpDPdyFine:
        case spv::OpFwidthFine:
        case spv::OpDPdxCoarse:
        case spv::OpDPdyCoarse:
        case spv::OpFwidthCoarse:
        case spv::OpPhi:
        case spv::OpAtomicLoad:
        case spv::OpAtomicExchange:
        case spv::OpAtomicCompareExchange:
        case spv::OpAtomicCompareExchangeWeak:
        case spv::OpAtomicIIncrement:
        case spv::OpAtomicIDecrement:
        case spv::OpAtomicIAdd:
        case spv::OpAtomicISub:
        case spv::OpAtomicSMin:
        case spv::OpAtomicUMin:
        case spv::OpAtomicSMax:
        case spv::OpAtomicUMax:
        case spv::OpAtomicAnd:
        case spv::OpAtomicOr:
        case spv::OpAtomicXor:
        case spv::OpAtomicFlagTestAndSet:
        case spv::OpNamedBarrierInitialize:
        case spv::OpGroupAsyncCopy:
        case spv::OpGroupAll:
        case spv::OpGroupAny:
        case spv::OpGroupIAdd:
        case spv::OpGroupFAdd:
        case spv::OpGroupFMin:
        case spv::OpGroupUMin:
        case spv::OpGroupSMin:
        case spv::OpGroupFMax:
        case spv::OpGroupUMax:
        case spv::OpGroupSMax:
        case spv::OpSubgroupBallotKHR:
        case spv::OpSubgroupFirstInvocationKHR:
        case spv::OpSubgroupReadInvocationKHR:
        case spv::OpEnqueueMarker:
        case spv::OpEnqueueKernel:
        case spv::OpGetKernelNDrangeSubGroupCount:
        case spv::OpGetKernelNDrangeMaxSubGroupSize:
        case spv::OpGetKernelWorkGroupSize:
        case spv::OpGetKernelPreferredWorkGroupSizeMultiple:
        case spv::OpCreateUserEvent:
        case spv::OpIsValidEvent:
        case spv::OpGetDefaultQueue:
        case spv::OpBuildNDRange:
        case spv::OpGetKernelLocalSizeForSubgroupCount:
        case spv::OpGetKernelMaxNumSubgroups:
        case spv::OpReadPipe:
        case spv::OpWritePipe:
        case spv::OpReservedReadPipe:
        case spv::OpReservedWritePipe:
        case spv::OpReserveReadPipePackets:
        case spv::OpReserveWritePipePackets:
        case spv::OpIsValidReserveId:
        case spv::OpGetNumPipePackets:
        case spv::OpGetMaxPipePackets:
        case spv::OpGroupReserveReadPipePackets:
        case spv::OpGroupReserveWritePipePackets:
        case spv::OpConstantPipeStorage:
        case spv::OpCreatePipeFromPipeStorage:
            return true;
        default:
            return false;
    }
}

static bool HasSpirvInstructionResultId(const spv::Op opCode)
{
    switch (opCode)
    {
        case spv::OpUndef:
        case spv::OpSizeOf:
        case spv::OpString:
        case spv::OpDecorationGroup:
        case spv::OpExtInstImport:
        case spv::OpExtInst:
        case spv::OpTypeVoid:
        case spv::OpTypeBool:
        case spv::OpTypeInt:
        case spv::OpTypeFloat:
        case spv::OpTypeVector:
        case spv::OpTypeMatrix:
        case spv::OpTypeImage:
        case spv::OpTypeSampler:
        case spv::OpTypeSampledImage:
        case spv::OpTypeArray:
        case spv::OpTypeRuntimeArray:
        case spv::OpTypeStruct:
        case spv::OpTypeOpaque:
        case spv::OpTypePointer:
        case spv::OpTypeFunction:
        case spv::OpTypeEvent:
        case spv::OpTypeDeviceEvent:
        case spv::OpTypeReserveId:
        case spv::OpTypeQueue:
        case spv::OpTypePipe:
        case spv::OpTypePipeStorage:
        case spv::OpTypeNamedBarrier:
        case spv::OpConstantTrue:
        case spv::OpConstantFalse:
        case spv::OpConstant:
        case spv::OpConstantComposite:
        case spv::OpConstantSampler:
        case spv::OpConstantNull:
        case spv::OpSpecConstantTrue:
        case spv::OpSpecConstantFalse:
        case spv::OpSpecConstant:
        case spv::OpSpecConstantComposite:
        case spv::OpSpecConstantOp:
        case spv::OpVariable:
        case spv::OpImageTexelPointer:
        case spv::OpLoad:
        case spv::OpAccessChain:
        case spv::OpInBoundsAccessChain:
        case spv::OpPtrAccessChain:
        case spv::OpArrayLength:
        case spv::OpGenericPtrMemSemantics:
        case spv::OpInBoundsPtrAccessChain:
        case spv::OpFunction:
        case spv::OpFunctionParameter:
        case spv::OpFunctionCall:
        case spv::OpSampledImage:
        case spv::OpImageSampleImplicitLod:
        case spv::OpImageSampleExplicitLod:
        case spv::OpImageSampleDrefImplicitLod:
        case spv::OpImageSampleDrefExplicitLod:
        case spv::OpImageSampleProjImplicitLod:
        case spv::OpImageSampleProjExplicitLod:
        case spv::OpImageSampleProjDrefImplicitLod:
        case spv::OpImageSampleProjDrefExplicitLod:
        case spv::OpImageFetch:
        case spv::OpImageGather:
        case spv::OpImageDrefGather:
        case spv::OpImageRead:
        case spv::OpImage:
        case spv::OpImageQueryFormat:
        case spv::OpImageQueryOrder:
        case spv::OpImageQuerySizeLod:
        case spv::OpImageQuerySize:
        case spv::OpImageQueryLod:
        case spv::OpImageQueryLevels:
        case spv::OpImageQuerySamples:
        case spv::OpImageSparseSampleImplicitLod:
        case spv::OpImageSparseSampleExplicitLod:
        case spv::OpImageSparseSampleDrefImplicitLod:
        case spv::OpImageSparseSampleDrefExplicitLod:
        case spv::OpImageSparseSampleProjImplicitLod:
        case spv::OpImageSparseSampleProjExplicitLod:
        case spv::OpImageSparseSampleProjDrefImplicitLod:
        case spv::OpImageSparseSampleProjDrefExplicitLod:
        case spv::OpImageSparseFetch:
        case spv::OpImageSparseGather:
        case spv::OpImageSparseDrefGather:
        case spv::OpImageSparseTexelsResident:
        case spv::OpImageSparseRead:
        case spv::OpConvertFToU:
        case spv::OpConvertFToS:
        case spv::OpConvertSToF:
        case spv::OpConvertUToF:
        case spv::OpUConvert:
        case spv::OpSConvert:
        case spv::OpFConvert:
        case spv::OpQuantizeToF16:
        case spv::OpConvertPtrToU:
        case spv::OpSatConvertSToU:
        case spv::OpSatConvertUToS:
        case spv::OpConvertUToPtr:
        case spv::OpPtrCastToGeneric:
        case spv::OpGenericCastToPtr:
        case spv::OpGenericCastToPtrExplicit:
        case spv::OpBitcast:
        case spv::OpVectorExtractDynamic:
        case spv::OpVectorInsertDynamic:
        case spv::OpVectorShuffle:
        case spv::OpCompositeConstruct:
        case spv::OpCompositeExtract:
        case spv::OpCompositeInsert:
        case spv::OpCopyObject:
        case spv::OpTranspose:
        case spv::OpSNegate:
        case spv::OpFNegate:
        case spv::OpIAdd:
        case spv::OpFAdd:
        case spv::OpISub:
        case spv::OpFSub:
        case spv::OpIMul:
        case spv::OpFMul:
        case spv::OpUDiv:
        case spv::OpSDiv:
        case spv::OpFDiv:
        case spv::OpUMod:
        case spv::OpSRem:
        case spv::OpSMod:
        case spv::OpFRem:
        case spv::OpFMod:
        case spv::OpVectorTimesScalar:
        case spv::OpMatrixTimesScalar:
        case spv::OpVectorTimesMatrix:
        case spv::OpMatrixTimesVector:
        case spv::OpMatrixTimesMatrix:
        case spv::OpOuterProduct:
        case spv::OpDot:
        case spv::OpIAddCarry:
        case spv::OpISubBorrow:
        case spv::OpUMulExtended:
        case spv::OpSMulExtended:
        case spv::OpShiftRightLogical:
        case spv::OpShiftRightArithmetic:
        case spv::OpShiftLeftLogical:
        case spv::OpBitwiseOr:
        case spv::OpBitwiseXor:
        case spv::OpBitwiseAnd:
        case spv::OpNot:
        case spv::OpBitFieldInsert:
        case spv::OpBitFieldSExtract:
        case spv::OpBitFieldUExtract:
        case spv::OpBitReverse:
        case spv::OpBitCount:
        case spv::OpAny:
        case spv::OpAll:
        case spv::OpIsNan:
        case spv::OpIsInf:
        case spv::OpIsFinite:
        case spv::OpIsNormal:
        case spv::OpSignBitSet:
        case spv::OpLessOrGreater:
        case spv::OpOrdered:
        case spv::OpUnordered:
        case spv::OpLogicalEqual:
        case spv::OpLogicalNotEqual:
        case spv::OpLogicalOr:
        case spv::OpLogicalAnd:
        case spv::OpLogicalNot:
        case spv::OpSelect:
        case spv::OpIEqual:
        case spv::OpINotEqual:
        case spv::OpUGreaterThan:
        case spv::OpSGreaterThan:
        case spv::OpUGreaterThanEqual:
        case spv::OpSGreaterThanEqual:
        case spv::OpULessThan:
        case spv::OpSLessThan:
        case spv::OpULessThanEqual:
        case spv::OpSLessThanEqual:
        case spv::OpFOrdEqual:
        case spv::OpFUnordEqual:
        case spv::OpFOrdNotEqual:
        case spv::OpFUnordNotEqual:
        case spv::OpFOrdLessThan:
        case spv::OpFUnordLessThan:
        case spv::OpFOrdGreaterThan:
        case spv::OpFUnordGreaterThan:
        case spv::OpFOrdLessThanEqual:
        case spv::OpFUnordLessThanEqual:
        case spv::OpFOrdGreaterThanEqual:
        case spv::OpFUnordGreaterThanEqual:
        case spv::OpDPdx:
        case spv::OpDPdy:
        case spv::OpFwidth:
        case spv::OpDPdxFine:
        case spv::OpDPdyFine:
        case spv::OpFwidthFine:
        case spv::OpDPdxCoarse:
        case spv::OpDPdyCoarse:
        case spv::OpFwidthCoarse:
        case spv::OpPhi:
        case spv::OpLabel:
        case spv::OpAtomicLoad:
        case spv::OpAtomicExchange:
        case spv::OpAtomicCompareExchange:
        case spv::OpAtomicCompareExchangeWeak:
        case spv::OpAtomicIIncrement:
        case spv::OpAtomicIDecrement:
        case spv::OpAtomicIAdd:
        case spv::OpAtomicISub:
        case spv::OpAtomicSMin:
        case spv::OpAtomicUMin:
        case spv::OpAtomicSMax:
        case spv::OpAtomicUMax:
        case spv::OpAtomicAnd:
        case spv::OpAtomicOr:
        case spv::OpAtomicXor:
        case spv::OpAtomicFlagTestAndSet:
        case spv::OpNamedBarrierInitialize:
        case spv::OpGroupAsyncCopy:
        case spv::OpGroupAll:
        case spv::OpGroupAny:
        case spv::OpGroupIAdd:
        case spv::OpGroupFAdd:
        case spv::OpGroupFMin:
        case spv::OpGroupUMin:
        case spv::OpGroupSMin:
        case spv::OpGroupFMax:
        case spv::OpGroupUMax:
        case spv::OpGroupSMax:
        case spv::OpSubgroupBallotKHR:
        case spv::OpSubgroupFirstInvocationKHR:
        case spv::OpSubgroupReadInvocationKHR:
        case spv::OpEnqueueMarker:
        case spv::OpEnqueueKernel:
        case spv::OpGetKernelNDrangeSubGroupCount:
        case spv::OpGetKernelNDrangeMaxSubGroupSize:
        case spv::OpGetKernelWorkGroupSize:
        case spv::OpGetKernelPreferredWorkGroupSizeMultiple:
        case spv::OpCreateUserEvent:
        case spv::OpIsValidEvent:
        case spv::OpGetDefaultQueue:
        case spv::OpBuildNDRange:
        case spv::OpGetKernelLocalSizeForSubgroupCount:
        case spv::OpGetKernelMaxNumSubgroups:
        case spv::OpReadPipe:
        case spv::OpWritePipe:
        case spv::OpReservedReadPipe:
        case spv::OpReservedWritePipe:
        case spv::OpReserveReadPipePackets:
        case spv::OpReserveWritePipePackets:
        case spv::OpIsValidReserveId:
        case spv::OpGetNumPipePackets:
        case spv::OpGetMaxPipePackets:
        case spv::OpGroupReserveReadPipePackets:
        case spv::OpGroupReserveWritePipePackets:
        case spv::OpConstantPipeStorage:
        case spv::OpCreatePipeFromPipeStorage:
            return true;
        default:
            return false;
    }
}

SpirvInstructionInfo GetSpirvInstructionInfo(spv::Op opCode)
{
    SpirvInstructionInfo lookup;
    {
        lookup.hasType      = HasSpirvInstructionTypeId(opCode);
        lookup.hasResult    = HasSpirvInstructionResultId(opCode);
    }
    return lookup;
}

// SPIR-V generator magic numbers
// see https://www.khronos.org/registry/spir-v/api/spir-v.xml
const char* GetSpirvBuilderName(std::uint32_t builderMagic)
{
    switch (builderMagic)
    {
        case  0: return "Khronos";                              // Reserved by Khronos
        case  1: return "LunarG";                               // Contact TBD
        case  2: return "Valve";                                // Contact TBD
        case  3: return "Codeplay";                             // Contact Neil Henning, neil@codeplay.com
        case  4: return "NVIDIA";                               // Contact Kerch Holt, kholt@nvidia.com
        case  5: return "ARM";                                  // Contact Alexander Galazin, alexander.galazin@arm.com
        case  6: return "Khronos LLVM/SPIR-V Translator";       // Contact Yaxun (Sam) Liu, yaxun.liu@amd.com
        case  7: return "Khronos SPIR-V Tools Assembler";       // Contact David Neto, dneto@google.com
        case  8: return "Khronos Glslang Reference Front End";  // Contact John Kessenich, johnkessenich@google.com
        case  9: return "Qualcomm";                             // Contact weifengz@qti.qualcomm.com
        case 10: return "AMD";                                  // Contact Daniel Rakos, daniel.rakos@amd.com
        case 11: return "Intel";                                // Contact Alexey, alexey.bader@intel.com
        default: return "Unknown";
    }
}

const char* GetSpirvVersionString(std::uint32_t version)
{
    switch (version)
    {
        case 0x00010000: return "1.0";
        case 0x00010100: return "1.1";
        case 0x00010200: return "1.2";
        default:         return nullptr;
    }
}


} // /namespace LLGL



// ================================================================================
