/*
 * AMD64Assembler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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
        void WriteFuncCall(const void* addr, JITCallConv conv, bool farCall) override;

    private:

        struct Displacement;

        std::uint8_t DispMod(const Displacement& disp) const;
        std::uint8_t ModRM(std::uint8_t mode, Reg r0, Reg r1) const;

        void WritePrologue();
        void WriteEpilogue();

        void WriteStackFrame(
            const std::vector<JIT::ArgType>&    varArgTypes,
            const std::vector<std::uint32_t>&   stackChunks
        );

        void WriteOptREX(Reg reg, bool defaultsTo64Bit = false);
        void WriteOptDisp(const Displacement& disp);
        void WriteOptSIB(Reg reg);

        void BeginSupplement(const Arg& arg);
        void EndSupplement();
        void ApplySupplements();

        void ErrInvalidUseOfRSP();

    private:

        void PushReg(Reg srcReg);
        void PushImm8(std::uint8_t byte);
        void PushImm16(std::uint16_t word);
        void PushImm32(std::uint32_t dword);
        void Push(Reg srcReg);

        void PopReg(Reg dstReg);
        void Pop(Reg dstReg);

        void MovReg(Reg dstReg, Reg srcReg);
        void MovRegImm32(Reg dstReg, std::uint32_t dword);
        void MovRegImm64(Reg dstReg, std::uint64_t qword);
        void MovMemImm32(Reg dstMemReg, std::uint32_t dword, const Displacement& disp);
        void MovMemReg(Reg dstMemReg, Reg srcReg, const Displacement& disp);
        void MovRegMem(Reg dstReg, Reg srcMemReg, const Displacement& disp);

        #if 0 // UNUSED
        void MovSSRegMem(Reg dstReg, Reg srcMemReg, const Displacement& disp);
        void MovSDRegMem(Reg dstReg, Reg srcMemReg, const Displacement& disp);
        #endif // /UNUSED

        void MovSSRegImm32(Reg dstReg, float f32);
        void MovSDRegImm64(Reg dstReg, double f64);

        void MovDQURegMem(Reg dstReg, Reg srcMemReg, const Displacement& disp);
        void MovDQUMemReg(Reg dstMemReg, Reg srcReg, const Displacement& disp);

        void AddImm32(Reg dstReg, std::uint32_t dword);
        void SubImm32(Reg dstReg, std::uint32_t dword);
        void DivReg(Reg srcReg);
        void XOrReg(Reg dstReg, Reg srcReg);

        void CallNear(Reg reg);

        void RetNear(std::uint16_t word = 0);
        void RetFar(std::uint16_t word = 0);

        void Int(std::uint8_t byte);

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
            Displacement();
            Displacement(const Displacement&) = default;
            Displacement& operator = (const Displacement&) = default;

            bool has32Bits;
            union
            {
                std::int8_t     disp8;
                std::int32_t    disp32;
            };
        };

        struct Disp8 : public Displacement
        {
            Disp8(std::int8_t disp);
        };

        struct Disp32 : public Displacement
        {
            Disp32(std::int32_t disp);
        };

    private:

        std::uint32_t           	localStackSize_ = 0;
        std::uint16_t               paramStackSize_ = 0;
        Displacement                argStackBase_;

        // Supplement data that must be updated after encoding
        std::vector<Supplement>     supplements_;

        // Displacements of parameters within stack frame
        std::vector<Displacement>   varArgDisp_;

        // Base pointer offsets of stack allocations
        std::vector<std::uint32_t>  stackChunkOffsets_;

};


} // /namespace JIT

} // /namespace LLGL


#endif



// ================================================================================
