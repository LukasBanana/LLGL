/*
 * SPIRVReflect.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SPIRV_REFLECT_H
#define LLGL_SPIRV_REFLECT_H


#include "SPIRVParser.h"
#include <vector>
#include <map>


namespace LLGL
{


// SPIR-V shader module parser.
class SPIRVReflect : public SPIRVParser
{

    protected:

        struct Uniform
        {
            const char*     name            = nullptr;
            std::uint32_t   bindingPoint    = 0;
            std::uint32_t   descriptorSet   = 0;
        };

        struct Varying
        {
            const char*     name        = nullptr;
            std::uint32_t   location    = 0;
            bool            input       = false;
        };

    private:

        using Instr = SPIRVInstruction;

        void OnParseHeader(const SPIRVHeader& header) override;
        void OnParseInstruction(const SPIRVInstruction& instr) override;

        void OpName(const Instr& instr);
        void OpDecorate(const Instr& instr);
        void OpDecorateBinding(const Instr& instr);
        void OpDecorateLocation(const Instr& instr);

        void SetName(spv::Id id, const char* name);
        const char* GetName(spv::Id id) const;

        void AssertIdBound(spv::Id id) const;

    private:

        std::uint32_t               idBound_    = 0;
        std::vector<const char*>    names_;

        std::map<spv::Id, Uniform>  uniforms_;
        std::map<spv::Id, Varying>  varyings_;

};


} // /namespace LLGL


#endif



// ================================================================================
