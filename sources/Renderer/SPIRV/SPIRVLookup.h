/*
 * SPIRVLookup.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SPIRV_LOOKUP_H
#define LLGL_SPIRV_LOOKUP_H


#include <spirv/1.2/spirv.hpp11>
#include <cstdint>


namespace LLGL
{


struct SPIRVLookup
{
    bool hasType    = false;
    bool hasResult  = false;
};


// Returns the SPIR-V lookup information for the specified instruction opcode.
SPIRVLookup GetSPIRVLookup(spv::Op opCode);

// Returns the SPIR-V builder (or rather generator) name by the specified builder magic number, or null if the magic number is unknwon (see SPIRVHeader::builderMagic).
const char* GetSPIRVBuilderName(std::uint32_t builderMagic);

// Returns the specified SPIR-V version as string, or null if the version number is unknown.
const char* GetSPIRVVersionString(std::uint32_t version);


} // /namespace LLGL


#endif



// ================================================================================
