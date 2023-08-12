/*
 * SpirvInstruction.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SPIRV_INSTRUCTION_H
#define LLGL_SPIRV_INSTRUCTION_H


#include <spirv/1.2/spirv.hpp>
#include <cstdint>


namespace LLGL
{


// SPIR-V shader module instruction structure.
struct SpirvInstruction
{
    SpirvInstruction() = default;
    SpirvInstruction(const SpirvInstruction&) = default;

    SpirvInstruction(spv::Op opcode, spv::Id type = 0, spv::Id result = 0);
    SpirvInstruction(spv::Op opcode, spv::Id type, spv::Id result, std::uint32_t numOperands, const spv::Id* operands);

    // Constructs the instruction with a pointer to the data in a SPIR-V module.
    SpirvInstruction(const std::uint32_t* words);

    // Returns the specified operand as 32-bit unsigned integral value, or throws an out-of-bounds exception on failure.
    std::uint32_t GetUInt32(std::uint32_t operand) const;

    // Returns the specified operand as 64-bit unsigned integral value, or throws an out-of-bounds exception on failure.
    std::uint64_t GetUInt64(std::uint32_t operand) const;

    // Returns the specified operand as (decompressed) 16-bit floating-point value, or throws an out-of-bounds exception on failure.
    float GetFloat16(std::uint32_t operand) const;

    // Returns the specified operand as 32-bit floating-point value, or throws an out-of-bounds exception on failure.
    float GetFloat32(std::uint32_t operand) const;

    // Returns the specified operand as 64-bit floating-point value, or throws an out-of-bounds exception on failure.
    double GetFloat64(std::uint32_t operand) const;

    // Returns the operands as ASCII string with the specified offset, or throws an out-of-bounds exception on failure..
    const char* GetString(std::uint32_t operand) const;

    // Returns the operand offset after the end of the ASCII string operands beginning at the specified offset.
    std::uint32_t FindStringEndOperand(std::uint32_t firstOperand) const;

    spv::Op         opcode      = spv::OpNop;   // Instruction op-code. By default OpNop.
    spv::Id         type        = 0;            // Type ID number. By default 0.
    spv::Id         result      = 0;            // Result ID number. By default 0.
    std::uint32_t   numOperands = 0;
    const spv::Id*  operands    = nullptr;
};


} // /namespace LLGL


#endif



// ================================================================================
