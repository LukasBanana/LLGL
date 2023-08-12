/*
 * SpirvLookup.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SPIRV_INSTRUCTION_INFO_H
#define LLGL_SPIRV_INSTRUCTION_INFO_H


#include <spirv/1.2/spirv.hpp>
#include <cstdint>


namespace LLGL
{


struct SpirvInstructionInfo
{
    bool hasType    = false;
    bool hasResult  = false;
};


// Returns the SPIR-V lookup information for the specified instruction opcode.
SpirvInstructionInfo GetSpirvInstructionInfo(spv::Op opCode);

// Returns the SPIR-V builder (or rather generator) name by the specified builder magic number, or null if the magic number is unknwon (see SPIRVHeader::builderMagic).
const char* GetSpirvBuilderName(std::uint32_t builderMagic);

// Returns the specified SPIR-V version as string, or null if the version number is unknown.
const char* GetSpirvVersionString(std::uint32_t version);


} // /namespace LLGL


#endif



// ================================================================================
