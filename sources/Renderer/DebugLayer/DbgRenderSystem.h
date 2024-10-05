/*
 * DbgRenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_RENDER_SYSTEM_H
#define LLGL_DBG_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include <LLGL/RenderingDebugger.h>

#include "DbgSwapChain.h"
#include "DbgCommandBuffer.h"
#include "DbgCommandQueue.h"

#include "Buffer/DbgBuffer.h"
#include "Buffer/DbgBufferArray.h"
#include "RenderState/DbgPipelineLayout.h"
#include "RenderState/DbgPipelineState.h"
#include "RenderState/DbgQueryHeap.h"
#include "RenderState/DbgResourceHeap.h"
#include "RenderState/DbgRenderPass.h"
#include "Shader/DbgShader.h"
#include "Texture/DbgTexture.h"
#include "Texture/DbgRenderTarget.h"

#include "../ContainerTypes.h"


namespace LLGL
{


//TODO: move all validation functions into separate class to make them sharable between RenderSystem and other classes
class DbgRenderSystem final : public RenderSystem
{

    public:

        #include <LLGL/Backend/RenderSystem.inl>

    public:

        DbgRenderSystem(RenderSystemPtr&& instance, RenderingDebugger* debugger);

        void FlushProfile();

    private:

        #include <LLGL/Backend/RenderSystem.Internal.inl>

    private:

        void ValidateBindFlags(long flags);
        void ValidateCPUAccessFlags(long flags, long validFlags, const char* contextDesc = nullptr);
        void ValidateMiscFlags(long flags, long validFlags, const char* contextDesc = nullptr);
        void ValidateResourceCPUAccess(long cpuAccessFlags, const CPUAccess access, const char* resourceTypeName);

        void ValidateCommandBufferDesc(const CommandBufferDescriptor& commandBufferDesc);

        void ValidateBufferDesc(const BufferDescriptor& bufferDesc, std::uint32_t* formatSizeOut = nullptr);
        void ValidateVertexAttributesForBuffer(const VertexAttribute& lhs, const VertexAttribute& rhs);
        void ValidateBufferSize(std::uint64_t size);
        void ValidateConstantBufferSize(std::uint64_t size);
        void ValidateBufferBoundary(std::uint64_t bufferSize, std::uint64_t dstOffset, std::uint64_t dataSize);
        void ValidateBufferMapping(DbgBuffer& bufferDbg, bool mapMemory);
        void ValidateBufferView(DbgBuffer& bufferDbg, const BufferViewDescriptor& viewDesc, const BindingDescriptor& bindingDesc);

        void ValidateTextureDesc(const TextureDescriptor& textureDesc, const ImageView* initialImage = nullptr);
        void ValidateTextureFormatSupported(const Format format);
        void ValidateTextureDescMipLevels(const TextureDescriptor& textureDesc);
        void ValidateTextureSize(std::uint32_t size, std::uint32_t limit, const char* textureTypeName);
        void ValidateTextureSizePassiveDimension(std::uint32_t size, const char* textureTypeName, const char* axisName);
        void Validate1DTextureSize(std::uint32_t size);
        void Validate2DTextureSize(std::uint32_t size);
        void Validate3DTextureSize(std::uint32_t size);
        void ValidateCubeTextureSize(std::uint32_t width, std::uint32_t height);
        void ValidateArrayTextureLayers(const TextureType type, std::uint32_t layers);
        void ValidateMipLevelLimit(std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t maxNumMipLevels);
        void ValidateTextureArrayRange(const DbgTexture& textureDbg, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers);
        void ValidateTextureArrayRangeWithEnd(std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers, std::uint32_t arrayLayerLimit);
        void ValidateTextureRegion(const DbgTexture& textureDbg, const TextureRegion& textureRegion);
        void ValidateTextureView(const DbgTexture& sharedTextureDbg, const TextureViewDescriptor& textureViewDesc);
        void ValidateTextureViewType(const TextureType sharedTextureType, const TextureType textureViewType, const std::initializer_list<TextureType>& validTypes);
        void ValidateImageDataSize(const DbgTexture& textureDbg, const TextureRegion& textureRegion, ImageFormat imageFormat, DataType dataType, std::size_t dataSize);

        void ValidateAttachmentDesc(const AttachmentDescriptor& attachmentDesc, std::uint32_t colorTarget, bool isResolveAttachment, bool isDepthStencilAttachment);

        void ValidateShaderDesc(const ShaderDescriptor& shaderDesc);

        void ValidatePipelineLayoutDesc(const PipelineLayoutDescriptor& pipelineLayoutDesc);

        void ValidateResourceHeapDesc(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews);
        void ValidateResourceHeapRange(const DbgResourceHeap& resourceHeapDbg, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews);
        void ValidateResourceViewForBinding(const ResourceViewDescriptor& rvDesc, const BindingDescriptor& bindingDesc);
        void ValidateBufferForBinding(const DbgBuffer& bufferDbg, const BindingDescriptor& bindingDesc);
        void ValidateTextureForBinding(const DbgTexture& textureDbg, const BindingDescriptor& bindingDesc);

        void ValidateInputAssemblyDescriptor(const GraphicsPipelineDescriptor& pipelineStateDesc);
        void ValidateBlendTargetDescriptor(const BlendTargetDescriptor& blendTargetDesc, std::size_t idx);
        void ValidateBlendDescriptor(const BlendDescriptor& blendDesc, bool hasFragmentShader, bool hasDualSourceBlend);
        void ValidateGraphicsPipelineDesc(const GraphicsPipelineDescriptor& pipelineStateDesc);
        void ValidateComputePipelineDesc(const ComputePipelineDescriptor& pipelineStateDesc);
        void ValidateFragmentShaderOutput(DbgShader& fragmentShaderDbg, const RenderPass* renderPass, bool hasDualSourceBlend);
        void ValidateFragmentShaderOutputWithRenderPass(DbgShader& fragmentShaderDbg, const FragmentShaderAttributes& fragmentAttribs, const DbgRenderPass& renderPass, bool hasDualSourceBlend);
        void ValidateFragmentShaderOutputWithoutRenderPass(DbgShader& fragmentShaderDbg, const FragmentShaderAttributes& fragmentAttribs);
        void ValidatePipelineStateUniforms(const DbgPipelineLayout& pipelineLayout, const ArrayView<DbgShader*>& shaders);

        void Assert3DTextures();
        void AssertCubeTextures();
        void AssertArrayTextures();
        void AssertCubeArrayTextures();
        void AssertMultiSampleTextures();

        template <typename T, typename TBase>
        void ReleaseDbg(HWObjectContainer<T>& cont, TBase& entry);

        std::vector<ResourceViewDescriptor> GetResourceViewInstanceCopy(const ArrayView<ResourceViewDescriptor>& resourceViews);

    private:

        /* ----- Common objects ----- */

        RenderSystemPtr                         instance_;

        RenderingDebugger*                      debugger_   = nullptr;
        FrameProfile                            profile_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<DbgSwapChain>         swapChains_;
        HWObjectInstance<DbgCommandQueue>       commandQueue_;
        HWObjectContainer<DbgCommandBuffer>     commandBuffers_;
        HWObjectContainer<DbgBuffer>            buffers_;
        HWObjectContainer<DbgBufferArray>       bufferArrays_;
        HWObjectContainer<DbgTexture>           textures_;
        HWObjectContainer<DbgRenderPass>        renderPasses_;
        HWObjectContainer<DbgRenderTarget>      renderTargets_;
        HWObjectContainer<DbgShader>            shaders_;
        HWObjectContainer<DbgPipelineLayout>    pipelineLayouts_;
        HWObjectContainer<DbgPipelineState>     pipelineStates_;
        HWObjectContainer<DbgResourceHeap>      resourceHeaps_;
        //HWObjectContainer<DbgSampler>           samplers_;
        HWObjectContainer<DbgQueryHeap>         queryHeaps_;

};


} // /namespace LLGL


#endif



// ================================================================================
