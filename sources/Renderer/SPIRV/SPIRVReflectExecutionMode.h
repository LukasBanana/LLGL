/*
 * SPIRVReflectExecutionMode.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SPIRV_REFLECT_EXECUTION_MODE_H
#define LLGL_SPIRV_REFLECT_EXECUTION_MODE_H


#include "SPIRVParser.h"


namespace LLGL
{


// SPIR-V reflection class to read the execution mode.
class SPIRVReflectExecutionMode final : public SPIRVParser
{

    public:

        struct SpvExecutionMode
        {
            bool            earlyFragmentTest   = false;
            bool            originUpperLeft     = false;
            bool            depthGreater        = false;
            bool            depthLess           = false;
            std::uint32_t   localSizeX          = 0;
            std::uint32_t   localSizeY          = 0;
            std::uint32_t   localSizeZ          = 0;
        };

    public:

        // Returns the SPIR-V execution mode that was reflected after "Parse" was called.
        inline const SpvExecutionMode& GetMode() const
        {
            return mode_;
        }

    private:

        void OnParseHeader(const SPIRVHeader& header) override;
        void OnParseInstruction(const SPIRVInstruction& instr) override;

    private:

        void OnParseExecutionMode(const SPIRVInstruction& instr);

    private:

        SpvExecutionMode    mode_;
        bool                firstModeParsed_    = false;

};


} // /namespace LLGL


#endif



// ================================================================================
