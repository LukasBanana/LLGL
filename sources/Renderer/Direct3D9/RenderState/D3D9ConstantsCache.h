/*
 * D3D9ConstantsCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_CONSTANTS_CACHE_H
#define LLGL_D3D9_CONSTANTS_CACHE_H


#include "../Direct3D9.h"
#include "../Command/D3D9CommandOpcode.h"
#include <LLGL/Container/ArrayView.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <cstdint>


namespace LLGL
{


struct D3D9ShaderConstant;
struct D3D9ShaderConstantTable;

class D3D9ConstantsCache
{

    public:

        D3D9ConstantsCache(Shader* vs, Shader* ps, ArrayView<UniformDescriptor> uniformDescs);

        // Allocates any commands in the virtual command buffer needed to encode the current state of the constants cache.
        void AllocCommands(D3D9VirtualCommandBuffer& vcmdBuffer);

        // Invalidates the entire cache. This must be called when new vertex and pixel shaders are bound to the D3D device.
        void Invalidate();

        // Writes new constants data into the cache.
        HRESULT SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize);

    private:

        static constexpr std::uint32_t invalidOffset = 0xFFFFFFFF;

        enum D3DShaderStage
        {
            D3DShaderStage_Vertex,
            D3DShaderStage_Pixel,

            D3DShaderStage_Count,
        };

        enum class D3DConstantType
        {
            Float4,
            Int4,
            Bool4,
        };

        // Maps to D3D9 calls of SetVertexShaderConstantF etc.
        struct D3DConstantLayout
        {
            D3DConstantType type            :  2;   // Selects F/I/B for SetVertexShaderConstantF/I/B
            UINT            startRegister   : 15;   // First argument for SetVertexShaderConstantF etc.
            UINT            numVectors4     : 15;   // Third argument for SetVertexShaderConstantF etc.
        };

        struct D3DConstantStageLayout
        {
            std::vector<D3DConstantLayout>  constants;
            std::uint16_t                   invalidatedConstantRange[2] = { 0xFFFF, 0x0000 };

            inline bool IsDirty() const
            {
                return (invalidatedConstantRange[0] < invalidatedConstantRange[1]);
            }

            void Invalidate(std::uint32_t start, std::uint32_t count);
            void ClearCacheRange();
        };

        // Maps to LLGL calls of SetUniforms().
        struct ConstantLocation
        {
            std::uint32_t size            : 12; // Byte size
            std::uint32_t numVectors4VS   : 10; // Number of vectors for VS
            std::uint32_t numVectors4PS   : 10; // Number of vectors for PS
            std::uint32_t startRegisterVS : 16; // Start register for SetVertexShaderConstantF etc.
            std::uint32_t startRegisterPS : 16; // Start register for SetPixelShaderConstantF etc.
            std::uint32_t offsetVS;             // Byte offset starting from `constantRegisters_[startRegisterVS]`
            std::uint32_t offsetPS;             // Byte offset starting from `constantRegisters_[startRegisterPS]`
        };

        // Represents a single D3D constant register of four 32-bit words.
        struct ConstantRegister
        {
            std::uint32_t words[4];
        };

        struct ConstantNameContext
        {
            std::vector<std::string>            nameStack;                          // Stacks up struct members, e.g. "myStruct"."field0"."subField1"
            std::map<std::string, std::size_t>  constantLUT[D3DShaderStage_Count];  // Maps from name to D3DConstantStageLayout::constants index.
            std::uint32_t                       psStartOffset = 0;

            std::string FullName() const;
        };

    private:

        void BuildStageConstantRegisters(
            ConstantNameContext&            nameContext,
            D3DShaderStage                  stage,
            const D3D9ShaderConstantTable&  ctable
        );

        void BuildShaderConstantLayout(
            ConstantNameContext&        nameContext,
            D3DShaderStage              stage,
            const D3D9ShaderConstant&   inConstant
        );

        void BuildConstantLocation(
            const ConstantNameContext&  nameContext,
            const UniformDescriptor&    inUniformDesc,
            ConstantLocation&           outLocation
        );

        void AllocVertexShaderCommands(D3D9VirtualCommandBuffer& vcmdBuffer, const D3DConstantStageLayout& stageLayout);
        void AllocPixelShaderCommands(D3D9VirtualCommandBuffer& vcmdBuffer, const D3DConstantStageLayout& stageLayout);

        inline void* GetConstantPtrByOffset(std::uint32_t startRegister, std::uint32_t offset)
        {
            return GetConstantPtrByRegister<char>(startRegister) + offset;
        }

    private:

        template <typename T>
        T* GetConstantPtrByRegister(UINT startRegister)
        {
            return reinterpret_cast<T*>(&(constantRegisters_[startRegister].words[0]));
        }

        template <typename TCommand>
        void AllocSetShaderConstantCommand(D3D9VirtualCommandBuffer& vcmdBuffer, D3D9Opcode opcode, const D3DConstantLayout& layout, std::uint32_t registerOffset)
        {
            const std::size_t constantsSize = layout.numVectors4 * sizeof(ConstantRegister);
            TCommand* cmd = vcmdBuffer.AllocCommand<TCommand>(opcode, constantsSize);
            cmd->startRegister = layout.startRegister;
            cmd->vector4Count = layout.numVectors4;
            ::memcpy(cmd + 1, GetConstantPtrByRegister<void>(layout.startRegister + registerOffset), constantsSize);
        }

    private:

        std::vector<ConstantRegister>   constantRegisters_;
        std::uint32_t                   psRegisterOffset_                   = 0;
        D3DConstantStageLayout          stageLayouts_[D3DShaderStage_Count];
        std::vector<ConstantLocation>   constantsMap_;

};

using D3D9ConstantsCachePtr = std::unique_ptr<D3D9ConstantsCache>;


} // /namespace LLGL


#endif



// ================================================================================
