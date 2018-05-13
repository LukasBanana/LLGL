/*
 * SPIRVReflect.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SPIRVReflect.h"
#include <string>


namespace LLGL
{


void SPIRVReflect::OnParseHeader(const SPIRVHeader& header)
{
    idBound_ = header.idBound;
    if (idBound_ > 0)
        names_.resize(idBound_);
}

void SPIRVReflect::OnParseInstruction(const SPIRVInstruction& instr)
{
    switch (instr.opCode)
    {
        case spv::Op::OpName:
            OpName(instr);
            break;
        case spv::Op::OpDecorate:
            OpDecorate(instr);
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
        default:
            break;
    }
}

void SPIRVReflect::OpDecorateBinding(const Instr& instr)
{
    auto id         = instr.GetUInt32(0);
    auto& variable  = uniforms_[id];

    variable.name           = GetName(id);
    variable.bindingPoint   = instr.GetUInt32(2);
}

void SPIRVReflect::OpDecorateLocation(const Instr& instr)
{
    auto id         = instr.GetUInt32(0);
    auto& variable  = varyings_[id];

    variable.name       = GetName(id);
    variable.location   = instr.GetUInt32(2);
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


} // /namespace LLGL



// ================================================================================
