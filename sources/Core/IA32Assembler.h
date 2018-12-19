/*
 * IA32Assembler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_IA32_ASSEMBLER_H
#define LLGL_IA32_ASSEMBLER_H


#include <LLGL/Export.h>
#include <vector>
#include <cstdint>


namespace LLGL
{


enum class IA32Reg
{
    EAX,
    ECX,
    EDX,
    EBX,
    ESP,
    EBP,
    ESI,
    EDI,
};


// IA-32 (a.k.a. x86) assembly code generator.
class LLGL_EXPORT IA32Assembler
{

    public:

        inline const std::vector<std::uint8_t>& GetAssembly() const
        {
            return assembly_;
        }

        void PushReg(IA32Reg reg);
        void PushImm32(std::uint32_t dword);

        void PopReg(IA32Reg reg);

        void MovRegImm32(IA32Reg reg, std::uint32_t dword);

        void CallNear(IA32Reg reg);
        void CallFar(IA32Reg reg);

        void RetNear(std::uint16_t word = 0);
        void RetFar(std::uint16_t word = 0);

    private:

        void WriteByte(std::uint8_t byte);
        void WriteWord(std::uint16_t word);
        void WriteDWord(std::uint32_t dword);

    private:

        std::vector<std::uint8_t> assembly_;

};


} // /namespace LLGL


#endif



// ================================================================================
