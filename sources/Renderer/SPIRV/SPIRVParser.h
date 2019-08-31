/*
 * SPIRVParser.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SPIRV_PARSER_H
#define LLGL_SPIRV_PARSER_H


#include "SPIRVInstruction.h"
#include <cstddef>


namespace LLGL
{


// SPIR-V shader module header structure.
struct SPIRVHeader
{
    std::uint32_t spirvMagic;
    std::uint32_t spirvVersion;
    std::uint32_t builderMagic;
    std::uint32_t idBound;
    std::uint32_t schema;
};

// SPIR-V shader module parser.
class SPIRVParser
{

    public:

        virtual ~SPIRVParser() = default;

        // Parses the specified SPIR-V shader byte code and throws an std::invalid_argument exception if the byte code is invalid.
        void Parse(const void* byteCode, std::size_t byteCodeSize);

    protected:

        // Finishes the parsing process.
        void Finish();

        // Returns true if the parsing process has finished.
        bool HasFinished() const;

    protected:

        // Callback function for the SPIR-V shader module header.
        virtual void OnParseHeader(const SPIRVHeader& header);

        // Callback function for each instruction within the SPIR-V shader module.
        virtual void OnParseInstruction(const SPIRVInstruction& instr);

    private:

        bool finished_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
