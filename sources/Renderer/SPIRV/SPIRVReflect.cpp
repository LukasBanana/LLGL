/*
 * SPIRVReflect.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SPIRVReflect.h"
#include "../../Core/Helper.h"
#include <string>


namespace LLGL
{


const SPIRVReflect::SpvType* SPIRVReflect::SpvType::DereferencePtr() const
{
    auto type = this;

    while (type->opcode == spv::Op::OpTypePointer)
    {
        if (type->baseType != nullptr)
            type = type->baseType;
        else
            return nullptr;
    }

    return type;
}

const SPIRVReflect::SpvType* SPIRVReflect::SpvType::DereferencePtr(const spv::Op opcodeType) const
{
    if (auto type = DereferencePtr())
        return (type->opcode == opcodeType ? type : nullptr);
    else
        return nullptr;
}

bool SPIRVReflect::SpvType::RefersToType(const spv::Op opcodeType) const
{
    return (DereferencePtr(opcodeType) != nullptr);
}

void SPIRVReflect::OnParseHeader(const SPIRVHeader& header)
{
    idBound_ = header.idBound;
    if (idBound_ > 0)
        names_.resize(idBound_);
}

void SPIRVReflect::OnParseInstruction(const SPIRVInstruction& instr)
{
    switch (instr.opcode)
    {
        case spv::Op::OpName:
            OpName(instr);
            break;
        case spv::Op::OpDecorate:
            OpDecorate(instr);
            break;
        case spv::Op::OpTypeVoid:
        case spv::Op::OpTypeBool:
        case spv::Op::OpTypeInt:
        case spv::Op::OpTypeFloat:
        case spv::Op::OpTypeVector:
        case spv::Op::OpTypeMatrix:
        case spv::Op::OpTypeImage:
        case spv::Op::OpTypeSampler:
        case spv::Op::OpTypeSampledImage:
        case spv::Op::OpTypeArray:
        case spv::Op::OpTypeRuntimeArray:
        case spv::Op::OpTypeStruct:
        case spv::Op::OpTypeOpaque:
        case spv::Op::OpTypePointer:
        case spv::Op::OpTypeFunction:
            OpType(instr);
            break;
        case spv::Op::OpVariable:
            OpVariable(instr);
            break;
        case spv::Op::OpConstant:
            OpConstant(instr);
            break;
        default:
            break;
    }
}

void SPIRVReflect::OpName(const Instr& instr)
{
    SetName(instr.GetUInt32(0), instr.GetASCII(1));
}

void SPIRVReflect::OpDecorate(const Instr& instr)
{
    auto decoration = static_cast<spv::Decoration>(instr.GetUInt32(1));
    switch (decoration)
    {
        case spv::Decoration::Binding:
            OpDecorateBinding(instr);
            break;
        case spv::Decoration::Location:
            OpDecorateLocation(instr);
            break;
        case spv::Decoration::BuiltIn:
            OpDecorateBuiltin(instr);
            break;
        default:
            break;
    }
}

void SPIRVReflect::OpDecorateBinding(const Instr& instr)
{
    auto id         = instr.GetUInt32(0);
    auto& variable  = uniforms_[id];

    variable.name       = GetName(id);
    variable.binding    = instr.GetUInt32(2);
}

void SPIRVReflect::OpDecorateLocation(const Instr& instr)
{
    auto id         = instr.GetUInt32(0);
    auto& variable  = varyings_[id];

    variable.name       = GetName(id);
    variable.location   = instr.GetUInt32(2);
}

void SPIRVReflect::OpDecorateBuiltin(const Instr& instr)
{
    auto id         = instr.GetUInt32(0);
    auto& variable  = varyings_[id];

    variable.name       = GetName(id);
    variable.builtin    = static_cast<spv::BuiltIn>(instr.GetUInt32(2));
}

void SPIRVReflect::OpType(const Instr& instr)
{
    /* Register type and store it as current type to operate on */
    auto& type = types_[instr.result];
    {
        type.opcode = instr.opcode;
        type.result = instr.result;
        type.name   = GetName(instr.result);
    }

    /* Parse respective OpType* instruction */
    #define LLGL_OPTYPE_CASE_HANDLER(NAME)  \
        case spv::Op::NAME:                 \
            NAME(instr, type);              \
            break

    switch (instr.opcode)
    {
        LLGL_OPTYPE_CASE_HANDLER( OpTypeVoid         );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeBool         );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeInt          );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeFloat        );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeVector       );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeMatrix       );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeImage        );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeSampler      );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeSampledImage );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeArray        );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeRuntimeArray );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeStruct       );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeOpaque       );
        LLGL_OPTYPE_CASE_HANDLER( OpTypePointer      );
        LLGL_OPTYPE_CASE_HANDLER( OpTypeFunction     );
        default:
            break;
    }

    #undef LLGL_OPTYPE_CASE_HANDLER
}

void SPIRVReflect::OpTypeVoid(const Instr& /*instr*/, SpvType& type)
{
    // do nothing
}

void SPIRVReflect::OpTypeBool(const Instr& /*instr*/, SpvType& type)
{
    type.size = 1;
}

void SPIRVReflect::OpTypeInt(const Instr& instr, SpvType& type)
{
    type.size = (instr.GetUInt32(0) / 8);
    type.sign = (instr.GetUInt32(1) != 0);
}

void SPIRVReflect::OpTypeFloat(const Instr& instr, SpvType& type)
{
    type.size = (instr.GetUInt32(0) / 8);
}

void SPIRVReflect::OpTypeVector(const Instr& instr, SpvType& type)
{
    type.baseType   = FindType(instr.GetUInt32(0));
    type.elements   = instr.GetUInt32(1);
    type.size       = type.baseType->size * type.elements;
}

void SPIRVReflect::OpTypeMatrix(const Instr& instr, SpvType& type)
{
    type.baseType   = FindType(instr.GetUInt32(0));
    type.elements   = instr.GetUInt32(1);
    type.size       = type.baseType->size * type.elements;
}

void SPIRVReflect::OpTypeImage(const Instr& instr, SpvType& type)
{
}

void SPIRVReflect::OpTypeSampler(const Instr& instr, SpvType& type)
{
}

void SPIRVReflect::OpTypeSampledImage(const Instr& instr, SpvType& type)
{
}

void SPIRVReflect::OpTypeArray(const Instr& instr, SpvType& type)
{
    type.baseType = FindType(instr.GetUInt32(0));
    auto arrayVal = FindConstant(instr.GetUInt32(1));
    type.elements = arrayVal->i32;
}

void SPIRVReflect::OpTypeRuntimeArray(const Instr& instr, SpvType& type)
{
}

static void AccumulateSizeInVectorBoundary(std::uint32_t& size, std::uint32_t alignment, std::uint32_t appendix)
{
    /* Check if padding must be added first */
    if (size % alignment + appendix > alignment)
        size = GetAlignedSize(size, alignment);

    /* Accumulate next appendix */
    size += appendix;
}

void SPIRVReflect::OpTypeStruct(const Instr& instr, SpvType& type)
{
    type.fieldTypes.reserve(instr.numOperands);
    for (std::uint32_t i = 0; i < instr.numOperands; ++i)
    {
        auto fieldType = FindType(instr.GetUInt32(i));
        type.fieldTypes.push_back(fieldType);
        AccumulateSizeInVectorBoundary(type.size, 16, fieldType->size);
    }
    type.size = GetAlignedSize(type.size, 16u);
}

void SPIRVReflect::OpTypeOpaque(const Instr& instr, SpvType& type)
{
}

void SPIRVReflect::OpTypePointer(const Instr& instr, SpvType& type)
{
    type.storage    = static_cast<spv::StorageClass>(instr.GetUInt32(0));
    type.baseType   = FindType(instr.GetUInt32(1));
}

void SPIRVReflect::OpTypeFunction(const Instr& instr, SpvType& type)
{
}

/*
Example:
  %11 = OpTypeFloat     32                  float
  %12 = OpTypeVector    %11 4               vec4 -> float[4]
  %13 = OpTypeMatrix    %12 4               mat4 -> vec4[4]
  %14 = OpTypeStruct    %13 %13 %12 %11     struct S { mat4; mat4; vec4; float }
  %15 = OpTypePointer   Uniform %14         S*
  %16 = OpVariable      %15 Uniform         uniform S
*/
void SPIRVReflect::OpVariable(const Instr& instr)
{
    auto storage = static_cast<spv::StorageClass>(instr.GetUInt32(0));

    switch (storage)
    {
        case spv::StorageClass::Uniform:
        case spv::StorageClass::UniformConstant:
        //case spv::StorageClass::PushConstant:
        {
            auto& var = uniforms_[instr.result];
            {
                var.type = FindType(instr.type);
                if (auto structType = var.type->DereferencePtr(spv::Op::OpTypeStruct))
                {
                    if (var.name == nullptr || *var.name == '\0')
                        var.name = structType->name;
                    var.size = structType->size;
                }
                else
                    var.size = var.type->size;
            }
        }
        break;

        case spv::StorageClass::Input:
        {
            auto& var = varyings_[instr.result];
            {
                var.type    = FindType(instr.type);
                var.input   = true;
            }
        }
        break;

        case spv::StorageClass::Output:
        {
            auto& var = varyings_[instr.result];
            {
                var.type    = FindType(instr.type);
                var.input   = false;
            }
        }
        break;

        default:
        break;
    }
}

void SPIRVReflect::OpConstant(const Instr& instr)
{
    auto& val = constants_[instr.result];
    {
        val.type = FindType(instr.type);

        if (val.type->opcode == spv::Op::OpTypeInt)
        {
            if (val.type->size == 2 || val.type->size == 4)
                val.u32 = instr.GetUInt32(0);
            else if (val.type->size == 8)
                val.u64 = instr.GetUInt64(0);
        }
        else if (val.type->opcode == spv::Op::OpTypeFloat)
        {
            if (val.type->size == 2)
                val.f32 = instr.GetFloat16(0);
            else if (val.type->size == 4)
                val.f32 = instr.GetFloat32(0);
            else if (val.type->size == 8)
                val.f64 = instr.GetFloat64(0);
        }
    }
}

void SPIRVReflect::SetName(spv::Id id, const char* name)
{
    AssertIdBound(id);
    names_[id] = name;
}

const char* SPIRVReflect::GetName(spv::Id id) const
{
    AssertIdBound(id);
    return names_[id];
}

void SPIRVReflect::AssertIdBound(spv::Id id) const
{
    if (id >= idBound_)
    {
        throw std::runtime_error(
            "ID number in SPIR-V shader module out of range (ID " + std::to_string(id) +
            " exceeded ID-bound of " + std::to_string(idBound_) + ")"
        );
    }
}

const SPIRVReflect::SpvType* SPIRVReflect::FindType(spv::Id id) const
{
    auto it = types_.find(id);
    if (it == types_.end())
        throw std::runtime_error("cannot find SPIR-V OpType* instruction with result ID %" + std::to_string(id));
    return &(it->second);
}

const SPIRVReflect::SpvConstant* SPIRVReflect::FindConstant(spv::Id id) const
{
    auto it = constants_.find(id);
    if (it == constants_.end())
        throw std::runtime_error("cannot find SPIR-V OpConstant instruction with with result ID %" + std::to_string(id));
    return &(it->second);
}


} // /namespace LLGL



// ================================================================================
