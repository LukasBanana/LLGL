/*
 * AMD64Assembler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_AMD64_ASSEMBLER_H
#define LLGL_AMD64_ASSEMBLER_H


#include "AMD64Register.h"
#include "../../JITCompiler.h"
#include <vector>
#include <cstdint>


namespace LLGL
{

namespace JIT
{


// AMD64 (a.k.a. x86_64) assembly code generator.
class AMD64Assembler final : public JITCompiler
{

    public:
    
        void Begin() override;
        void End() override;

    private:

        bool IsLittleEndian() const override;
        void WriteFuncCall(const void* addr, const JITCallConv conv, bool farCall) override;

    private:
    
        struct Displacement;

        std::uint8_t DispMod(const Displacement& disp) const;
        std::uint8_t ModRM(std::uint8_t mode, const Reg r0, const Reg r1) const;
    
        void WriteOptREX(const Reg reg, bool defaultsTo64Bit = false);
        void WriteOptDisp(const Displacement& disp);
    
        void BeginSupplement(const Arg& arg);
        void EndSupplement();
        void ApplySupplements();
    
        void ErrInvalidUseOfRSP();
    
    private:
    
        void PushReg(const Reg srcReg);
        void PushImm8(std::uint8_t byte);
        void PushImm16(std::uint16_t word);
        void PushImm32(std::uint32_t dword);
        void Push(const Reg srcReg);

        void PopReg(const Reg dstReg);
        void Pop(const Reg dstReg);

        void MovReg(const Reg dstReg, const Reg srcReg);
        void MovRegImm32(const Reg dstReg, std::uint32_t dword);
        void MovRegImm64(const Reg dstReg, std::uint64_t qword);
        void MovMemImm32(const Reg dstMemReg, std::uint32_t dword, const Displacement& disp);
        void MovMemReg(const Reg dstMemReg, const Reg srcReg, const Displacement& disp);
    
        #if 0 // UNUSED
        void MovSSRegMem(const Reg dstReg, const Reg srcMemReg, const Displacement& disp);
        void MovSDRegMem(const Reg dstReg, const Reg srcMemReg, const Displacement& disp);
        #endif // /UNUSED
    
        void MovSSRegImm32(const Reg dstReg, float f32);
        void MovSDRegImm64(const Reg dstReg, double f64);
    
        void MovDQURegMem(const Reg dstReg, const Reg srcMemReg, const Displacement& disp);
        void MovDQUMemReg(const Reg dstMemReg, const Reg srcReg, const Displacement& disp);

        void AddImm32(const Reg dst, std::uint32_t dword);
        void SubImm32(const Reg dst, std::uint32_t dword);
        void DivReg(const Reg src);

        void CallNear(const Reg reg);

        void RetNear(std::uint16_t word = 0);
        void RetFar(std::uint16_t word = 0);
    
        #if 1//TESTING
        void _ForceExcep();
        #endif
    
    private:
    
        struct Supplement
        {
            QWord           data;       // Supplement data to be written at the end of the program (e.g. float literals)
            std::uint8_t    dataSize;   // Data size (in bytes)
            std::uint64_t   rip;        // Program counter (RIP register)
            std::size_t     dstOffset;  // Destination byte offset where the instruction must be updated
        };
    
        struct Displacement
        {
            Displacement(std::int8_t disp8);
            Displacement(std::int32_t disp32);
            
            bool has32Bits;
            union
            {
                std::int8_t     disp8;
                std::int32_t    disp32;
            };
        };
    
    private:
    
        std::uint32_t           localStack_     = 0;
        std::vector<Supplement> supplements_;
    
};


} // /namespace JIT

} // /namespace LLGL


#endif



// ================================================================================
