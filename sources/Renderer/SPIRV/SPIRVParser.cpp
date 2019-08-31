/*
 * SPIRVParser.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "SPIRVParser.h"
#include "SPIRVLookup.h"
#include "../../Core/Helper.h"
#include <stdexcept>


namespace LLGL
{


void SPIRVParser::Parse(const void* byteCode, std::size_t byteCodeSize)
{
    finished_ = false;

    if (!byteCode)
        throw std::invalid_argument("SPIR-V shader byte code must not be a null pointer");
    if (byteCodeSize % 4 != 0)
        throw std::invalid_argument("size of SPIR-V shader byte code must be a multiple of 4 bytes");

    /* Convert byte code pointer to word pointer */
    auto words      = reinterpret_cast<const std::uint32_t*>(byteCode);
    auto numWords   = static_cast<std::uint32_t>(byteCodeSize / 4);

    if (numWords < 5)
        throw std::invalid_argument("too few words in SPIR-V shader module");

    /* Parse header */
    SPIRVHeader header;
    {
        header.spirvMagic   = words[0];
        header.spirvVersion = words[1];
        header.builderMagic = words[2];
        header.idBound      = words[3];
        header.schema       = words[4];
    }
    OnParseHeader(header);

    /* Parse instructions */
    for (std::uint32_t i = 5; i < numWords && !finished_;)
    {
        /* Parse next instruction */
        SPIRVInstruction instr;
        {
            /* Read word count and opcode */
            auto firstWord = words[i++];

            auto wordCount  = (firstWord >> spv::WordCountShift);
            instr.opcode    = static_cast<spv::Op>(firstWord & spv::OpCodeMask);

            --wordCount;

            auto lookup = GetSPIRVLookup(instr.opcode);

            /* Read type (if used) */
            if (wordCount > 0 && lookup.hasType)
            {
                instr.type = words[i++];
                --wordCount;
            }

            /* Read result (if used) */
            if (wordCount > 0 && lookup.hasResult)
            {
                instr.result = words[i++];
                --wordCount;
            }

            /* Read operands */
            if (wordCount > 0)
            {
                instr.operands      = &(words[i]);
                instr.numOperands   = wordCount;
                i += wordCount;
            }
        }
        OnParseInstruction(instr);
    }

    finished_ = true;
}


/*
 * ======= Protected: =======
 */

void SPIRVParser::Finish()
{
    finished_ = true;
}

bool SPIRVParser::HasFinished() const
{
    return finished_;
}

void SPIRVParser::OnParseHeader(const SPIRVHeader& header)
{
    /* Validate SPIR-V magic number */
    if (header.spirvMagic != spv::MagicNumber)
    {
        throw std::invalid_argument(
            "invalid magic number in SPIR-V shader module (expected 0x" +
            ToHex(spv::MagicNumber) + ", but got 0x" + ToHex(header.spirvMagic) + ")"
        );
    }
}

void SPIRVParser::OnParseInstruction(const SPIRVInstruction& instr)
{
    // dummy
}


} // /namespace LLGL



// ================================================================================
