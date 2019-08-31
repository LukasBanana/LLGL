/*
 * SPIRVLookup.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SPIRVLookup.h"


namespace LLGL
{

namespace SPIRVHelper
{


using Op = spv::Op;

static bool HasTypeId(const Op opCode)
{
    switch (opCode)
    {
        case Op::OpUndef:
        case Op::OpSizeOf:
        case Op::OpMemberName:
        case Op::OpExtInst:
        case Op::OpTypeForwardPointer:
        case Op::OpConstantTrue:
        case Op::OpConstantFalse:
        case Op::OpConstant:
        case Op::OpConstantComposite:
        case Op::OpConstantSampler:
        case Op::OpConstantNull:
        case Op::OpSpecConstantTrue:
        case Op::OpSpecConstantFalse:
        case Op::OpSpecConstant:
        case Op::OpSpecConstantComposite:
        case Op::OpSpecConstantOp:
        case Op::OpVariable:
        case Op::OpImageTexelPointer:
        case Op::OpLoad:
        case Op::OpAccessChain:
        case Op::OpInBoundsAccessChain:
        case Op::OpPtrAccessChain:
        case Op::OpArrayLength:
        case Op::OpGenericPtrMemSemantics:
        case Op::OpInBoundsPtrAccessChain:
        case Op::OpFunction:
        case Op::OpFunctionParameter:
        case Op::OpFunctionCall:
        case Op::OpSampledImage:
        case Op::OpImageSampleImplicitLod:
        case Op::OpImageSampleExplicitLod:
        case Op::OpImageSampleDrefImplicitLod:
        case Op::OpImageSampleDrefExplicitLod:
        case Op::OpImageSampleProjImplicitLod:
        case Op::OpImageSampleProjExplicitLod:
        case Op::OpImageSampleProjDrefImplicitLod:
        case Op::OpImageSampleProjDrefExplicitLod:
        case Op::OpImageFetch:
        case Op::OpImageGather:
        case Op::OpImageDrefGather:
        case Op::OpImageRead:
        case Op::OpImage:
        case Op::OpImageQueryFormat:
        case Op::OpImageQueryOrder:
        case Op::OpImageQuerySizeLod:
        case Op::OpImageQuerySize:
        case Op::OpImageQueryLod:
        case Op::OpImageQueryLevels:
        case Op::OpImageQuerySamples:
        case Op::OpImageSparseSampleImplicitLod:
        case Op::OpImageSparseSampleExplicitLod:
        case Op::OpImageSparseSampleDrefImplicitLod:
        case Op::OpImageSparseSampleDrefExplicitLod:
        case Op::OpImageSparseSampleProjImplicitLod:
        case Op::OpImageSparseSampleProjExplicitLod:
        case Op::OpImageSparseSampleProjDrefImplicitLod:
        case Op::OpImageSparseSampleProjDrefExplicitLod:
        case Op::OpImageSparseFetch:
        case Op::OpImageSparseGather:
        case Op::OpImageSparseDrefGather:
        case Op::OpImageSparseTexelsResident:
        case Op::OpImageSparseRead:
        case Op::OpConvertFToU:
        case Op::OpConvertFToS:
        case Op::OpConvertSToF:
        case Op::OpConvertUToF:
        case Op::OpUConvert:
        case Op::OpSConvert:
        case Op::OpFConvert:
        case Op::OpQuantizeToF16:
        case Op::OpConvertPtrToU:
        case Op::OpSatConvertSToU:
        case Op::OpSatConvertUToS:
        case Op::OpConvertUToPtr:
        case Op::OpPtrCastToGeneric:
        case Op::OpGenericCastToPtr:
        case Op::OpGenericCastToPtrExplicit:
        case Op::OpBitcast:
        case Op::OpVectorExtractDynamic:
        case Op::OpVectorInsertDynamic:
        case Op::OpVectorShuffle:
        case Op::OpCompositeConstruct:
        case Op::OpCompositeExtract:
        case Op::OpCompositeInsert:
        case Op::OpCopyObject:
        case Op::OpTranspose:
        case Op::OpSNegate:
        case Op::OpFNegate:
        case Op::OpIAdd:
        case Op::OpFAdd:
        case Op::OpISub:
        case Op::OpFSub:
        case Op::OpIMul:
        case Op::OpFMul:
        case Op::OpUDiv:
        case Op::OpSDiv:
        case Op::OpFDiv:
        case Op::OpUMod:
        case Op::OpSRem:
        case Op::OpSMod:
        case Op::OpFRem:
        case Op::OpFMod:
        case Op::OpVectorTimesScalar:
        case Op::OpMatrixTimesScalar:
        case Op::OpVectorTimesMatrix:
        case Op::OpMatrixTimesVector:
        case Op::OpMatrixTimesMatrix:
        case Op::OpOuterProduct:
        case Op::OpDot:
        case Op::OpIAddCarry:
        case Op::OpISubBorrow:
        case Op::OpUMulExtended:
        case Op::OpSMulExtended:
        case Op::OpShiftRightLogical:
        case Op::OpShiftRightArithmetic:
        case Op::OpShiftLeftLogical:
        case Op::OpBitwiseOr:
        case Op::OpBitwiseXor:
        case Op::OpBitwiseAnd:
        case Op::OpNot:
        case Op::OpBitFieldInsert:
        case Op::OpBitFieldSExtract:
        case Op::OpBitFieldUExtract:
        case Op::OpBitReverse:
        case Op::OpBitCount:
        case Op::OpAny:
        case Op::OpAll:
        case Op::OpIsNan:
        case Op::OpIsInf:
        case Op::OpIsFinite:
        case Op::OpIsNormal:
        case Op::OpSignBitSet:
        case Op::OpLessOrGreater:
        case Op::OpOrdered:
        case Op::OpUnordered:
        case Op::OpLogicalEqual:
        case Op::OpLogicalNotEqual:
        case Op::OpLogicalOr:
        case Op::OpLogicalAnd:
        case Op::OpLogicalNot:
        case Op::OpSelect:
        case Op::OpIEqual:
        case Op::OpINotEqual:
        case Op::OpUGreaterThan:
        case Op::OpSGreaterThan:
        case Op::OpUGreaterThanEqual:
        case Op::OpSGreaterThanEqual:
        case Op::OpULessThan:
        case Op::OpSLessThan:
        case Op::OpULessThanEqual:
        case Op::OpSLessThanEqual:
        case Op::OpFOrdEqual:
        case Op::OpFUnordEqual:
        case Op::OpFOrdNotEqual:
        case Op::OpFUnordNotEqual:
        case Op::OpFOrdLessThan:
        case Op::OpFUnordLessThan:
        case Op::OpFOrdGreaterThan:
        case Op::OpFUnordGreaterThan:
        case Op::OpFOrdLessThanEqual:
        case Op::OpFUnordLessThanEqual:
        case Op::OpFOrdGreaterThanEqual:
        case Op::OpFUnordGreaterThanEqual:
        case Op::OpDPdx:
        case Op::OpDPdy:
        case Op::OpFwidth:
        case Op::OpDPdxFine:
        case Op::OpDPdyFine:
        case Op::OpFwidthFine:
        case Op::OpDPdxCoarse:
        case Op::OpDPdyCoarse:
        case Op::OpFwidthCoarse:
        case Op::OpPhi:
        case Op::OpAtomicLoad:
        case Op::OpAtomicExchange:
        case Op::OpAtomicCompareExchange:
        case Op::OpAtomicCompareExchangeWeak:
        case Op::OpAtomicIIncrement:
        case Op::OpAtomicIDecrement:
        case Op::OpAtomicIAdd:
        case Op::OpAtomicISub:
        case Op::OpAtomicSMin:
        case Op::OpAtomicUMin:
        case Op::OpAtomicSMax:
        case Op::OpAtomicUMax:
        case Op::OpAtomicAnd:
        case Op::OpAtomicOr:
        case Op::OpAtomicXor:
        case Op::OpAtomicFlagTestAndSet:
        case Op::OpNamedBarrierInitialize:
        case Op::OpGroupAsyncCopy:
        case Op::OpGroupAll:
        case Op::OpGroupAny:
        case Op::OpGroupIAdd:
        case Op::OpGroupFAdd:
        case Op::OpGroupFMin:
        case Op::OpGroupUMin:
        case Op::OpGroupSMin:
        case Op::OpGroupFMax:
        case Op::OpGroupUMax:
        case Op::OpGroupSMax:
        case Op::OpSubgroupBallotKHR:
        case Op::OpSubgroupFirstInvocationKHR:
        case Op::OpSubgroupReadInvocationKHR:
        case Op::OpEnqueueMarker:
        case Op::OpEnqueueKernel:
        case Op::OpGetKernelNDrangeSubGroupCount:
        case Op::OpGetKernelNDrangeMaxSubGroupSize:
        case Op::OpGetKernelWorkGroupSize:
        case Op::OpGetKernelPreferredWorkGroupSizeMultiple:
        case Op::OpCreateUserEvent:
        case Op::OpIsValidEvent:
        case Op::OpGetDefaultQueue:
        case Op::OpBuildNDRange:
        case Op::OpGetKernelLocalSizeForSubgroupCount:
        case Op::OpGetKernelMaxNumSubgroups:
        case Op::OpReadPipe:
        case Op::OpWritePipe:
        case Op::OpReservedReadPipe:
        case Op::OpReservedWritePipe:
        case Op::OpReserveReadPipePackets:
        case Op::OpReserveWritePipePackets:
        case Op::OpIsValidReserveId:
        case Op::OpGetNumPipePackets:
        case Op::OpGetMaxPipePackets:
        case Op::OpGroupReserveReadPipePackets:
        case Op::OpGroupReserveWritePipePackets:
        case Op::OpConstantPipeStorage:
        case Op::OpCreatePipeFromPipeStorage:
            return true;
        default:
            return false;
    }
}

static bool HasResultId(const Op opCode)
{
    switch (opCode)
    {
        case Op::OpUndef:
        case Op::OpSizeOf:
        case Op::OpString:
        case Op::OpDecorationGroup:
        case Op::OpExtInstImport:
        case Op::OpExtInst:
        case Op::OpTypeVoid:
        case Op::OpTypeBool:
        case Op::OpTypeInt:
        case Op::OpTypeFloat:
        case Op::OpTypeVector:
        case Op::OpTypeMatrix:
        case Op::OpTypeImage:
        case Op::OpTypeSampler:
        case Op::OpTypeSampledImage:
        case Op::OpTypeArray:
        case Op::OpTypeRuntimeArray:
        case Op::OpTypeStruct:
        case Op::OpTypeOpaque:
        case Op::OpTypePointer:
        case Op::OpTypeFunction:
        case Op::OpTypeEvent:
        case Op::OpTypeDeviceEvent:
        case Op::OpTypeReserveId:
        case Op::OpTypeQueue:
        case Op::OpTypePipe:
        case Op::OpTypePipeStorage:
        case Op::OpTypeNamedBarrier:
        case Op::OpConstantTrue:
        case Op::OpConstantFalse:
        case Op::OpConstant:
        case Op::OpConstantComposite:
        case Op::OpConstantSampler:
        case Op::OpConstantNull:
        case Op::OpSpecConstantTrue:
        case Op::OpSpecConstantFalse:
        case Op::OpSpecConstant:
        case Op::OpSpecConstantComposite:
        case Op::OpSpecConstantOp:
        case Op::OpVariable:
        case Op::OpImageTexelPointer:
        case Op::OpLoad:
        case Op::OpAccessChain:
        case Op::OpInBoundsAccessChain:
        case Op::OpPtrAccessChain:
        case Op::OpArrayLength:
        case Op::OpGenericPtrMemSemantics:
        case Op::OpInBoundsPtrAccessChain:
        case Op::OpFunction:
        case Op::OpFunctionParameter:
        case Op::OpFunctionCall:
        case Op::OpSampledImage:
        case Op::OpImageSampleImplicitLod:
        case Op::OpImageSampleExplicitLod:
        case Op::OpImageSampleDrefImplicitLod:
        case Op::OpImageSampleDrefExplicitLod:
        case Op::OpImageSampleProjImplicitLod:
        case Op::OpImageSampleProjExplicitLod:
        case Op::OpImageSampleProjDrefImplicitLod:
        case Op::OpImageSampleProjDrefExplicitLod:
        case Op::OpImageFetch:
        case Op::OpImageGather:
        case Op::OpImageDrefGather:
        case Op::OpImageRead:
        case Op::OpImage:
        case Op::OpImageQueryFormat:
        case Op::OpImageQueryOrder:
        case Op::OpImageQuerySizeLod:
        case Op::OpImageQuerySize:
        case Op::OpImageQueryLod:
        case Op::OpImageQueryLevels:
        case Op::OpImageQuerySamples:
        case Op::OpImageSparseSampleImplicitLod:
        case Op::OpImageSparseSampleExplicitLod:
        case Op::OpImageSparseSampleDrefImplicitLod:
        case Op::OpImageSparseSampleDrefExplicitLod:
        case Op::OpImageSparseSampleProjImplicitLod:
        case Op::OpImageSparseSampleProjExplicitLod:
        case Op::OpImageSparseSampleProjDrefImplicitLod:
        case Op::OpImageSparseSampleProjDrefExplicitLod:
        case Op::OpImageSparseFetch:
        case Op::OpImageSparseGather:
        case Op::OpImageSparseDrefGather:
        case Op::OpImageSparseTexelsResident:
        case Op::OpImageSparseRead:
        case Op::OpConvertFToU:
        case Op::OpConvertFToS:
        case Op::OpConvertSToF:
        case Op::OpConvertUToF:
        case Op::OpUConvert:
        case Op::OpSConvert:
        case Op::OpFConvert:
        case Op::OpQuantizeToF16:
        case Op::OpConvertPtrToU:
        case Op::OpSatConvertSToU:
        case Op::OpSatConvertUToS:
        case Op::OpConvertUToPtr:
        case Op::OpPtrCastToGeneric:
        case Op::OpGenericCastToPtr:
        case Op::OpGenericCastToPtrExplicit:
        case Op::OpBitcast:
        case Op::OpVectorExtractDynamic:
        case Op::OpVectorInsertDynamic:
        case Op::OpVectorShuffle:
        case Op::OpCompositeConstruct:
        case Op::OpCompositeExtract:
        case Op::OpCompositeInsert:
        case Op::OpCopyObject:
        case Op::OpTranspose:
        case Op::OpSNegate:
        case Op::OpFNegate:
        case Op::OpIAdd:
        case Op::OpFAdd:
        case Op::OpISub:
        case Op::OpFSub:
        case Op::OpIMul:
        case Op::OpFMul:
        case Op::OpUDiv:
        case Op::OpSDiv:
        case Op::OpFDiv:
        case Op::OpUMod:
        case Op::OpSRem:
        case Op::OpSMod:
        case Op::OpFRem:
        case Op::OpFMod:
        case Op::OpVectorTimesScalar:
        case Op::OpMatrixTimesScalar:
        case Op::OpVectorTimesMatrix:
        case Op::OpMatrixTimesVector:
        case Op::OpMatrixTimesMatrix:
        case Op::OpOuterProduct:
        case Op::OpDot:
        case Op::OpIAddCarry:
        case Op::OpISubBorrow:
        case Op::OpUMulExtended:
        case Op::OpSMulExtended:
        case Op::OpShiftRightLogical:
        case Op::OpShiftRightArithmetic:
        case Op::OpShiftLeftLogical:
        case Op::OpBitwiseOr:
        case Op::OpBitwiseXor:
        case Op::OpBitwiseAnd:
        case Op::OpNot:
        case Op::OpBitFieldInsert:
        case Op::OpBitFieldSExtract:
        case Op::OpBitFieldUExtract:
        case Op::OpBitReverse:
        case Op::OpBitCount:
        case Op::OpAny:
        case Op::OpAll:
        case Op::OpIsNan:
        case Op::OpIsInf:
        case Op::OpIsFinite:
        case Op::OpIsNormal:
        case Op::OpSignBitSet:
        case Op::OpLessOrGreater:
        case Op::OpOrdered:
        case Op::OpUnordered:
        case Op::OpLogicalEqual:
        case Op::OpLogicalNotEqual:
        case Op::OpLogicalOr:
        case Op::OpLogicalAnd:
        case Op::OpLogicalNot:
        case Op::OpSelect:
        case Op::OpIEqual:
        case Op::OpINotEqual:
        case Op::OpUGreaterThan:
        case Op::OpSGreaterThan:
        case Op::OpUGreaterThanEqual:
        case Op::OpSGreaterThanEqual:
        case Op::OpULessThan:
        case Op::OpSLessThan:
        case Op::OpULessThanEqual:
        case Op::OpSLessThanEqual:
        case Op::OpFOrdEqual:
        case Op::OpFUnordEqual:
        case Op::OpFOrdNotEqual:
        case Op::OpFUnordNotEqual:
        case Op::OpFOrdLessThan:
        case Op::OpFUnordLessThan:
        case Op::OpFOrdGreaterThan:
        case Op::OpFUnordGreaterThan:
        case Op::OpFOrdLessThanEqual:
        case Op::OpFUnordLessThanEqual:
        case Op::OpFOrdGreaterThanEqual:
        case Op::OpFUnordGreaterThanEqual:
        case Op::OpDPdx:
        case Op::OpDPdy:
        case Op::OpFwidth:
        case Op::OpDPdxFine:
        case Op::OpDPdyFine:
        case Op::OpFwidthFine:
        case Op::OpDPdxCoarse:
        case Op::OpDPdyCoarse:
        case Op::OpFwidthCoarse:
        case Op::OpPhi:
        case Op::OpLabel:
        case Op::OpAtomicLoad:
        case Op::OpAtomicExchange:
        case Op::OpAtomicCompareExchange:
        case Op::OpAtomicCompareExchangeWeak:
        case Op::OpAtomicIIncrement:
        case Op::OpAtomicIDecrement:
        case Op::OpAtomicIAdd:
        case Op::OpAtomicISub:
        case Op::OpAtomicSMin:
        case Op::OpAtomicUMin:
        case Op::OpAtomicSMax:
        case Op::OpAtomicUMax:
        case Op::OpAtomicAnd:
        case Op::OpAtomicOr:
        case Op::OpAtomicXor:
        case Op::OpAtomicFlagTestAndSet:
        case Op::OpNamedBarrierInitialize:
        case Op::OpGroupAsyncCopy:
        case Op::OpGroupAll:
        case Op::OpGroupAny:
        case Op::OpGroupIAdd:
        case Op::OpGroupFAdd:
        case Op::OpGroupFMin:
        case Op::OpGroupUMin:
        case Op::OpGroupSMin:
        case Op::OpGroupFMax:
        case Op::OpGroupUMax:
        case Op::OpGroupSMax:
        case Op::OpSubgroupBallotKHR:
        case Op::OpSubgroupFirstInvocationKHR:
        case Op::OpSubgroupReadInvocationKHR:
        case Op::OpEnqueueMarker:
        case Op::OpEnqueueKernel:
        case Op::OpGetKernelNDrangeSubGroupCount:
        case Op::OpGetKernelNDrangeMaxSubGroupSize:
        case Op::OpGetKernelWorkGroupSize:
        case Op::OpGetKernelPreferredWorkGroupSizeMultiple:
        case Op::OpCreateUserEvent:
        case Op::OpIsValidEvent:
        case Op::OpGetDefaultQueue:
        case Op::OpBuildNDRange:
        case Op::OpGetKernelLocalSizeForSubgroupCount:
        case Op::OpGetKernelMaxNumSubgroups:
        case Op::OpReadPipe:
        case Op::OpWritePipe:
        case Op::OpReservedReadPipe:
        case Op::OpReservedWritePipe:
        case Op::OpReserveReadPipePackets:
        case Op::OpReserveWritePipePackets:
        case Op::OpIsValidReserveId:
        case Op::OpGetNumPipePackets:
        case Op::OpGetMaxPipePackets:
        case Op::OpGroupReserveReadPipePackets:
        case Op::OpGroupReserveWritePipePackets:
        case Op::OpConstantPipeStorage:
        case Op::OpCreatePipeFromPipeStorage:
            return true;
        default:
            return false;
    }
}


} // /namespace SPIRVHelper


SPIRVLookup GetSPIRVLookup(spv::Op opCode)
{
    SPIRVLookup lookup;
    {
        lookup.hasType      = SPIRVHelper::HasTypeId(opCode);
        lookup.hasResult    = SPIRVHelper::HasResultId(opCode);
    }
    return lookup;
}

// SPIR-V generator magic numbers
// see https://www.khronos.org/registry/spir-v/api/spir-v.xml
const char* GetSPIRVBuilderName(std::uint32_t builderMagic)
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

const char* GetSPIRVVersionString(std::uint32_t version)
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
