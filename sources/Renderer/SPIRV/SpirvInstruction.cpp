/*
 * SpirvInstruction.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "SpirvInstruction.h"
#include "SpirvInstructionInfo.h"
#include "SpirvIterator.h"
#include "../../Core/Assertion.h"
#include "../../Core/Float16Compressor.h"
#include <stdexcept>


namespace LLGL
{


SpirvInstruction::SpirvInstruction(spv::Op opcode, spv::Id type, spv::Id result) :
    opcode { opcode },
    type   { type   },
    result { result }
{
}

SpirvInstruction::SpirvInstruction(spv::Op opcode, spv::Id type, spv::Id result, std::uint32_t numOperands, const spv::Id* operands) :
    opcode      { opcode      },
    type        { type        },
    result      { result      },
    numOperands { numOperands },
    operands    { operands    }
{
}

SpirvInstruction::SpirvInstruction(const std::uint32_t* words) :
    opcode { SpirvConstForwardIterator{ words }.Opcode() }
{
    std::uint32_t wordCount = SpirvConstForwardIterator{ words }.WordCount();

    /* Read type (if used) */
    const SpirvInstructionInfo info = GetSpirvInstructionInfo(opcode);
    std::uint32_t operandOffset = 1;

    if (wordCount > operandOffset && info.hasType)
        type = words[operandOffset++];

    /* Read result (if used) */
    if (wordCount > operandOffset && info.hasResult)
        result = words[operandOffset++];

    /* Read operands */
    if (wordCount > operandOffset)
    {
        operands    = words + operandOffset;
        numOperands = wordCount - operandOffset;
    }
}

std::uint32_t SpirvInstruction::GetUInt32(std::uint32_t operand) const
{
    LLGL_ASSERT(operand < numOperands);
    return operands[operand];
}

std::uint64_t SpirvInstruction::GetUInt64(std::uint32_t operand) const
{
    /* Extract 64-bit integral */
    LLGL_ASSERT(operand + 1 < numOperands);
    std::uint64_t ui = 0;

    ui = operands[operand];
    ui <<= 32;
    ui |= operands[operand + 1];

    return ui;
}

float SpirvInstruction::GetFloat16(std::uint32_t operand) const
{
    return DecompressFloat16(static_cast<std::uint16_t>(GetUInt32(operand)));
}

float SpirvInstruction::GetFloat32(std::uint32_t operand) const
{
    /* Extract 32-bit floating-point from 32-bit integer operands */
    LLGL_ASSERT(operand < numOperands);
    return *reinterpret_cast<const float*>(&(operands[operand]));
}

double SpirvInstruction::GetFloat64(std::uint32_t operand) const
{
    /* Extract 64-bit floating-point from 64-bit integer operand */
    const std::uint64_t ui = GetUInt64(operand);
    return *reinterpret_cast<const double*>(&ui);
}

const char* SpirvInstruction::GetString(std::uint32_t operand) const
{
    LLGL_ASSERT(operand < numOperands);
    return reinterpret_cast<const char*>(&(operands[operand]));
}

std::uint32_t SpirvInstruction::FindStringEndOperand(std::uint32_t operand) const
{
    for (; operand < numOperands; ++operand)
    {
        /* Check for null terminator in current word */
        spv::Id word = operands[operand];
        for (int i = 0; i < 4; ++i)
        {
            /* Check if current byte is zero */
            if ((word & 0xFF) == 0)
                return operand + 1;

            /* Shift word to check next byte */
            word >>= 8;
        }
    }
    return operand;
}


} // /namespace LLGL



// ================================================================================
