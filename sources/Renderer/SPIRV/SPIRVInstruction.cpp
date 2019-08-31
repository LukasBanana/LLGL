/*
 * SPIRVInstruction.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SPIRVInstruction.h"
#include "../../Core/Float16Compressor.h"
#include <stdexcept>


namespace LLGL
{


[[noreturn]]
static void ErrOperandIndexOutOfRange()
{
    throw std::out_of_range("operand index in SPIR-V instruction out of range");
}

SPIRVInstruction::SPIRVInstruction(spv::Op opcode, spv::Id type, spv::Id result) :
    opcode { opcode },
    type   { type   },
    result { result }
{
}

SPIRVInstruction::SPIRVInstruction(spv::Op opcode, spv::Id type, spv::Id result, std::uint32_t numOperands, const spv::Id* operands) :
    opcode      { opcode      },
    type        { type        },
    result      { result      },
    numOperands { numOperands },
    operands    { operands    }
{
}

std::uint32_t SPIRVInstruction::GetUInt32(std::uint32_t offset) const
{
    if (offset < numOperands)
        return operands[offset];
    else
        ErrOperandIndexOutOfRange();
}

std::uint64_t SPIRVInstruction::GetUInt64(std::uint32_t offset) const
{
    if (offset + 1 < numOperands)
    {
        /* Extract 64-bit integral */
        std::uint64_t ui = 0;

        ui = operands[offset];
        ui <<= 32;
        ui |= operands[offset + 1];

        return ui;
    }
    else
        ErrOperandIndexOutOfRange();
}

float SPIRVInstruction::GetFloat16(std::uint32_t offset) const
{
    return DecompressFloat16(static_cast<std::uint16_t>(GetUInt32(offset)));
}

float SPIRVInstruction::GetFloat32(std::uint32_t offset) const
{
    if (offset < numOperands)
    {
        /* Extract 32-bit floating-point */
        union
        {
            std::uint32_t   ui;
            float           f;
        }
        data;

        data.ui = operands[offset];

        return data.f;
    }
    else
        ErrOperandIndexOutOfRange();
}

double SPIRVInstruction::GetFloat64(std::uint32_t offset) const
{
    if (offset + 1 < numOperands)
    {
        /* Extract 32-bit floating-point */
        union
        {
            std::uint64_t   ui;
            double          f;
        }
        data;

        data.ui = operands[offset];
        data.ui <<= 32;
        data.ui |= operands[offset + 1];

        return data.f;
    }
    else
        ErrOperandIndexOutOfRange();
}

const char* SPIRVInstruction::GetASCII(std::uint32_t offset) const
{
    if (offset < numOperands)
        return reinterpret_cast<const char*>(&(operands[offset]));
    else
        ErrOperandIndexOutOfRange();
}

std::uint32_t SPIRVInstruction::FindASCIIEndOffset(std::uint32_t offset) const
{
    for (; offset < numOperands; ++offset)
    {
        /* Check for null terminator in current word */
        auto word = operands[offset];
        for (int i = 0; i < 4; ++i)
        {
            /* Check if current byte is zero */
            if ((word & 0xff) == 0)
                return offset + 1;

            /* Shift word to check next byte */
            word >>= 8;
        }
    }
    return offset;
}


} // /namespace LLGL



// ================================================================================
