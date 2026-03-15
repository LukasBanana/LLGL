/*
 * D3D9ConstantsCache.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9ConstantsCache.h"
#include "D3D9PipelineLayout.h"
#include "../Command/D3D9Command.h"
#include "../Shader/D3D9VertexShader.h"
#include "../Shader/D3D9PixelShader.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <cstdint>
#include <string.h>


namespace LLGL
{


static const D3D9ShaderConstantTable* GetD3D9ConstantTable(Shader* shader)
{
    if (shader != nullptr)
    {
        D3D9Shader* shaderD3D = LLGL_CAST(D3D9Shader*, shader);
        return &(shaderD3D->GetConstantTable());
    }
    return nullptr;
}

D3D9ConstantsCache::D3D9ConstantsCache(Shader* vs, Shader* ps, ArrayView<UniformDescriptor> uniformDescs)
{
    /* Build constant register layouts for each stage */
    ConstantNameContext nameContext;
    if (const D3D9ShaderConstantTable* vsCtable = GetD3D9ConstantTable(vs))
    {
        BuildStageConstantRegisters(nameContext, D3DShaderStage_Vertex, *vsCtable);
        psRegisterOffset_ = static_cast<std::uint32_t>(constantRegisters_.size());
        nameContext.psStartOffset = static_cast<std::uint32_t>(psRegisterOffset_ * sizeof(ConstantRegister));
    }
    if (const D3D9ShaderConstantTable* psCtable = GetD3D9ConstantTable(ps))
        BuildStageConstantRegisters(nameContext, D3DShaderStage_Pixel, *psCtable);

    /* Build containers to map from uniform to constant registers */
    constantsMap_.resize(uniformDescs.size(), {});
    for_range(i, uniformDescs.size())
        BuildConstantLocation(nameContext, uniformDescs[i], constantsMap_[i]);
}

void D3D9ConstantsCache::Invalidate()
{
    for_range(stage, D3DShaderStage_Count)
        stageLayouts_[stage].Invalidate(0, static_cast<std::uint16_t>(stageLayouts_[stage].constants.size()));
}

void D3D9ConstantsCache::AllocCommands(D3D9VirtualCommandBuffer& vcmdBuffer)
{
    if (stageLayouts_[D3DShaderStage_Vertex].IsDirty())
    {
        AllocVertexShaderCommands(vcmdBuffer, stageLayouts_[D3DShaderStage_Vertex]);
        stageLayouts_[D3DShaderStage_Vertex].ClearCacheRange();
    }
    if (stageLayouts_[D3DShaderStage_Pixel].IsDirty())
    {
        AllocPixelShaderCommands(vcmdBuffer, stageLayouts_[D3DShaderStage_Pixel]);
        stageLayouts_[D3DShaderStage_Pixel].ClearCacheRange();
    }
}

//TODO: this should be optimized by sharing memory space between VS and PS if they point to the same registers
HRESULT D3D9ConstantsCache::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    for (auto* dataByteAligned = static_cast<const char*>(data); dataSize > 0; ++first)
    {
        if (first >= constantsMap_.size())
            return E_INVALIDARG;

        /* Get current uniform location with its cbuffer */
        const D3D9ConstantsCache::ConstantLocation& location = constantsMap_[first];
        const std::uint16_t chunkSize = std::min<std::uint16_t>(dataSize, static_cast<std::uint16_t>(location.size));

        /* Copy input data into cache and move to next uniform */
        if (location.offsetVS != invalidOffset)
        {
            const std::uint32_t chunkSizeVS = std::min<std::uint32_t>(chunkSize, location.numVectors4VS * 16);
            ::memcpy(GetConstantPtrByOffset(location.startRegisterVS, /*location.offsetVS*/0), dataByteAligned, chunkSizeVS);
            //stageLayouts_[D3DShaderStage_Vertex].Invalidate(location.startRegisterVS, location.numVectors4VS);
            Invalidate();
        }
        if (location.offsetPS != invalidOffset)
        {
            const std::uint32_t chunkSizePS = std::min<std::uint32_t>(chunkSize, location.numVectors4PS * 16);
            ::memcpy(GetConstantPtrByOffset(psRegisterOffset_ + location.startRegisterPS, /*location.offsetPS*/0), dataByteAligned, chunkSizePS);
            //stageLayouts_[D3DShaderStage_Pixel].Invalidate(location.startRegisterPS, location.numVectors4PS);
            Invalidate();
        }

        dataByteAligned += chunkSize;
        dataSize -= chunkSize;
    }
    return S_OK;
}


/*
 * ======= Private: =======
 */

void D3D9ConstantsCache::BuildStageConstantRegisters(
    ConstantNameContext&            nameContext,
    D3DShaderStage                  stage,
    const D3D9ShaderConstantTable&  ctable)
{
    for (const D3D9ShaderConstant& constant : ctable.constants)
        BuildShaderConstantLayout(nameContext, stage, constant);
}

void D3D9ConstantsCache::BuildShaderConstantLayout(
    ConstantNameContext&        nameContext,
    D3DShaderStage              stage,
    const D3D9ShaderConstant&   inConstant)
{
    if (inConstant.type == D3D9UniformType::Undefined)
        return;

    nameContext.nameStack.push_back(inConstant.name);

    auto ToD3DConstantType = [](D3D9UniformType type) -> D3DConstantType
        {
            switch (type)
            {
                case D3D9UniformType::Bool:     return D3DConstantType::Bool4;
                case D3D9UniformType::Int:      return D3DConstantType::Int4;
                case D3D9UniformType::Float:    return D3DConstantType::Float4;
            }
            LLGL_UNREACHABLE();
        };

#if 0
    if (inConstant.type == D3D9UniformType::Undefined)
    {
        /* Build sub-constants for struct members */
        for (const D3D9ShaderConstant& structMember : inConstant.structMembers)
            BuildShaderConstantLayout(nameContext, stage, structMember);
    }
    else
#endif
    {
        std::vector<D3DConstantLayout>& constantLayouts = stageLayouts_[stage].constants;

        if (inConstant.reg.component == 0)
        {
            const std::size_t constantIndex = constantLayouts.size();

            /* Add new constant layout */
            D3DConstantLayout newConstant;
            {
                newConstant.type            = ToD3DConstantType(inConstant.type);
                newConstant.startRegister   = inConstant.reg.index;
                newConstant.numVectors4     = inConstant.reg.count;
            }
            constantLayouts.push_back(newConstant);

            /* Add full name for the new constant layout */
            nameContext.constantLUT[stage].insert({ nameContext.FullName(), constantIndex });
        }
        else
        {
            /* Register with component offset must share their memory with a previously allocated register */
            LLGL_ASSERT(
                inConstant.reg.count == 1,
                "cannot handle shader constant with component offset (%d) and size of more than one register (%d)",
                static_cast<int>(inConstant.reg.component), static_cast<int>(inConstant.reg.count)
            );
        }

        /* Allocate new registers as needed */
        const std::size_t numShaderRegisters = psRegisterOffset_ + inConstant.reg.index + inConstant.reg.count;
        constantRegisters_.resize(std::max<std::size_t>(constantRegisters_.size(), numShaderRegisters));
    }

    nameContext.nameStack.pop_back();
}

void D3D9ConstantsCache::BuildConstantLocation(
    const ConstantNameContext&      nameContext,
    const UniformDescriptor&        inUniformDesc,
    ConstantLocation&               outLocation)
{
    constexpr std::uint32_t d3dRegisterSize = sizeof(float) * 4;

    auto GetD3DConstantSize = [&inUniformDesc](const D3DConstantLayout& constantLayout) -> std::uint32_t
    {
        if (inUniformDesc.type == UniformType::Undefined)
            return GetUniformTypeSize(UniformType::Float4, constantLayout.numVectors4);
        else
            return GetUniformTypeSize(inUniformDesc.type, inUniformDesc.arraySize);
    };

    /* Store location to vertex constant */
    {
        const auto& constantLUT = nameContext.constantLUT[D3DShaderStage_Vertex];
        auto it = constantLUT.find(inUniformDesc.name.c_str());
        if (it != constantLUT.end())
        {
            const D3DConstantLayout& constantLayout = stageLayouts_[D3DShaderStage_Vertex].constants[it->second];

            outLocation.size            = GetD3DConstantSize(constantLayout);
            outLocation.numVectors4VS   = constantLayout.numVectors4;
            outLocation.offsetVS        = constantLayout.startRegister * d3dRegisterSize;
            outLocation.startRegisterVS = constantLayout.startRegister;
        }
        else
            outLocation.offsetVS = invalidOffset;
    }

    /* Store location to pixel constant */
    {
        const auto& constantLUT = nameContext.constantLUT[D3DShaderStage_Pixel];
        auto it = constantLUT.find(inUniformDesc.name.c_str());
        if (it != constantLUT.end())
        {
            const D3DConstantLayout& constantLayout = stageLayouts_[D3DShaderStage_Pixel].constants[it->second];

            outLocation.size            = std::max<std::uint32_t>(outLocation.size, GetD3DConstantSize(constantLayout));
            outLocation.numVectors4PS   = constantLayout.numVectors4;
            outLocation.offsetPS        = nameContext.psStartOffset + constantLayout.startRegister * d3dRegisterSize;
            outLocation.startRegisterPS = constantLayout.startRegister;
        }
        else
            outLocation.offsetPS = invalidOffset;
    }
}

void D3D9ConstantsCache::AllocVertexShaderCommands(D3D9VirtualCommandBuffer& vcmdBuffer, const D3DConstantStageLayout& stageLayout)
{
    for_subrange(i, stageLayout.invalidatedConstantRange[0], stageLayout.invalidatedConstantRange[1])
    {
        const D3DConstantLayout& layout = stageLayout.constants[i];
        switch (layout.type)
        {
            case D3DConstantType::Float4:
                AllocSetShaderConstantCommand<D3D9CmdSetShaderConstant>(vcmdBuffer, D3D9OpcodeSetVertexShaderConstantF, layout, 0);
                break;
            case D3DConstantType::Int4:
                AllocSetShaderConstantCommand<D3D9CmdSetShaderConstant>(vcmdBuffer, D3D9OpcodeSetVertexShaderConstantI, layout, 0);
                break;
            case D3DConstantType::Bool4:
                AllocSetShaderConstantCommand<D3D9CmdSetShaderConstant>(vcmdBuffer, D3D9OpcodeSetVertexShaderConstantB, layout, 0);
                break;
        }
    }
}

void D3D9ConstantsCache::AllocPixelShaderCommands(D3D9VirtualCommandBuffer& vcmdBuffer, const D3DConstantStageLayout& stageLayout)
{
    for_subrange(i, stageLayout.invalidatedConstantRange[0], stageLayout.invalidatedConstantRange[1])
    {
        const D3DConstantLayout& layout = stageLayout.constants[i];
        switch (layout.type)
        {
            case D3DConstantType::Float4:
                AllocSetShaderConstantCommand<D3D9CmdSetShaderConstant>(vcmdBuffer, D3D9OpcodeSetPixelShaderConstantF, layout, psRegisterOffset_);
                break;
            case D3DConstantType::Int4:
                AllocSetShaderConstantCommand<D3D9CmdSetShaderConstant>(vcmdBuffer, D3D9OpcodeSetPixelShaderConstantI, layout, psRegisterOffset_);
                break;
            case D3DConstantType::Bool4:
                AllocSetShaderConstantCommand<D3D9CmdSetShaderConstant>(vcmdBuffer, D3D9OpcodeSetPixelShaderConstantB, layout, psRegisterOffset_);
                break;
        }
    }
}


/*
 * D3DConstantStageLayout structure
 */

void D3D9ConstantsCache::D3DConstantStageLayout::Invalidate(std::uint32_t start, std::uint32_t count)
{
    invalidatedConstantRange[0] = static_cast<std::uint16_t>(std::min<std::uint32_t>(invalidatedConstantRange[0], start));
    invalidatedConstantRange[1] = static_cast<std::uint16_t>(std::max<std::uint32_t>(invalidatedConstantRange[1], start + count));
}

void D3D9ConstantsCache::D3DConstantStageLayout::ClearCacheRange()
{
    invalidatedConstantRange[0] = 0xFFFF;
    invalidatedConstantRange[1] = 0;
}


/*
 * ConstantNameContext structure
 */

std::string D3D9ConstantsCache::ConstantNameContext::FullName() const
{
    std::string outName;
    for_range(i, nameStack.size())
    {
        if (i > 0)
            outName.append(1, '.');
        outName.append(nameStack[i]);
    }
    return outName;
}


} // /namespace LLGL



// ================================================================================
