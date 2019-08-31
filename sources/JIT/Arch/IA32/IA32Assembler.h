/*
 * IA32Assembler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_IA32_ASSEMBLER_H
#define LLGL_IA32_ASSEMBLER_H


#include "IA32Register.h"
#include "../../JITCompiler.h"
#include <vector>
#include <cstdint>


namespace LLGL
{

namespace JIT
{


// IA-32 (a.k.a. x86) assembly code generator.
class IA32Assembler final : public JITCompiler
{

    public:

        void Begin() override;
        void End() override;

    private:

        bool IsLittleEndian() const override;
        void WriteFuncCall(const void* addr, JITCallConv conv, bool farCall) override;

    private:

        void PushReg(const Reg reg);
        void PushImm32(std::uint32_t dword);

        void PopReg(const Reg reg);

        void MovRegImm32(const Reg reg, std::uint32_t dword);

        void CallNear(const Reg reg);
        void CallFar(const Reg reg);

        void RetNear(std::uint16_t word = 0);
        void RetFar(std::uint16_t word = 0);

};


} // /namespace JIT

} // /namespace LLGL


#endif



// ================================================================================
