/*
 * SPIRVReflectExecutionMode.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SPIRVReflectExecutionMode.h"


namespace LLGL
{


void SPIRVReflectExecutionMode::OnParseHeader(const SPIRVHeader& header)
{
    firstModeParsed_ = false;
    if (header.spirvMagic != spv::MagicNumber)
        Finish();
}

void SPIRVReflectExecutionMode::OnParseInstruction(const SPIRVInstruction& instr)
{
    if (instr.opcode == spv::Op::OpExecutionMode)
    {
        OnParseExecutionMode(instr);
        firstModeParsed_ = true;
    }
    else if (firstModeParsed_)
    {
        /* Stop parsing after all OpExecutionMode instructions have been parsed */
        Finish();
    }
}

void SPIRVReflectExecutionMode::OnParseExecutionMode(const SPIRVInstruction& instr)
{
    auto mode = static_cast<spv::ExecutionMode>(instr.GetUInt32(1));
    switch (mode)
    {
        case spv::ExecutionMode::EarlyFragmentTests:
            mode_.earlyFragmentTest = true;
            break;

        case spv::ExecutionMode::OriginUpperLeft:
            mode_.originUpperLeft = true;
            break;

        case spv::ExecutionMode::DepthGreater:
            mode_.depthGreater = true;
            break;

        case spv::ExecutionMode::DepthLess:
            mode_.depthLess = true;
            break;

        case spv::ExecutionMode::LocalSize:
            mode_.localSizeX = instr.GetUInt32(2);
            mode_.localSizeY = instr.GetUInt32(3);
            mode_.localSizeZ = instr.GetUInt32(4);
            break;

        default:
            break;
    }
}


} // /namespace LLGL



// ================================================================================
