/*
 * SPIRVInstruction.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SPIRV_INSTRUCTION_H
#define LLGL_SPIRV_INSTRUCTION_H


#include <spirv/1.2/spirv.hpp11>
#include <cstdint>


namespace LLGL
{


// SPIR-V shader module instruction structure.
struct SPIRVInstruction
{
    SPIRVInstruction() = default;
    SPIRVInstruction(const SPIRVInstruction&) = default;

    SPIRVInstruction(spv::Op opCode, spv::Id type = 0, spv::Id result = 0);
    SPIRVInstruction(spv::Op opCode, spv::Id type, spv::Id result, std::uint32_t numOperands, const spv::Id* operands);

    // Returns the specified operand as 32-bit unsigned integral value, or throws an out-of-bounds exception on failure.
    std::uint32_t GetOperandUInt32(std::uint32_t offset) const;

    // Returns the specified operand as 64-bit unsigned integral value, or throws an out-of-bounds exception on failure.
    std::uint64_t GetOperandUInt64(std::uint32_t offset) const;

    // Returns the specified operand as (decompressed) 16-bit floating-point value, or throws an out-of-bounds exception on failure.
    float GetOperandFloat16(std::uint32_t offset) const;

    // Returns the specified operand as 32-bit floating-point value, or throws an out-of-bounds exception on failure.
    float GetOperandFloat32(std::uint32_t offset) const;

    // Returns the specified operand as 64-bit floating-point value, or throws an out-of-bounds exception on failure.
    double GetOperandFloat64(std::uint32_t offset) const;

    // Returns the operands as ASCII string with the specified offset, or throws an out-of-bounds exception on failure..
    const char* GetOperandASCII(std::uint32_t offset) const;

    // Returns the operand offset after the end of the ASCII string operands beginning at the specified offset.
    std::uint32_t FindOperandASCIIEndOffset(std::uint32_t offset) const;

    spv::Op         opCode      = spv::Op::OpNop;   // Instruction op-code. By default OpNop.
    spv::Id         type        = 0;                // Type ID number. By default 0.
    spv::Id         result      = 0;                // Result ID number. By default 0.
    std::uint32_t   numOperands = 0;
    const spv::Id*  operands    = nullptr;
};


} // /namespace LLGL


#endif



// ================================================================================
