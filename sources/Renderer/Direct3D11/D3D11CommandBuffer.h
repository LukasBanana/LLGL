/*
 * D3D11CommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_COMMAND_BUFFER_H
#define LLGL_D3D11_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <cstddef>
#include "../DXCommon/ComPtr.h"
#include "../DXCommon/DXCore.h"
#include <vector>
#include <d3d11.h>
#include <dxgi.h>


namespace LLGL
{


class D3D11StateManager;
class D3D11RenderTarget;

class D3D11CommandBuffer : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        D3D11CommandBuffer(D3D11StateManager& stateMngr, const ComPtr<ID3D11DeviceContext>& context);

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state) override;

        void SetViewport(const Viewport& viewport) override;
        void SetViewportArray(unsigned int numViewports, const Viewport* viewportArray) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissorArray(unsigned int numScissors, const Scissor* scissorArray) override;

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(int stencil) override;

        void Clear(long flags) override;
        void ClearTarget(unsigned int targetIndex, const LLGL::ColorRGBAf& color) override;

        /* ----- Buffers ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        void SetIndexBuffer(Buffer& buffer) override;
        
        void SetConstantBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetConstantBufferArray(BufferArray& bufferArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        
        void SetStorageBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetStorageBufferArray(BufferArray& bufferArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        void SetStreamOutputBuffer(Buffer& buffer) override;
        void SetStreamOutputBufferArray(BufferArray& bufferArray) override;

        void BeginStreamOutput(const PrimitiveType primitiveType) override;
        void EndStreamOutput() override;

        /* ----- Textures ----- */

        void SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetTextureArray(TextureArray& textureArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Sampler States ----- */

        void SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetSamplerArray(SamplerArray& samplerArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Render Targets ----- */

        void SetRenderTarget(RenderTarget& renderTarget) override;
        void SetRenderTarget(RenderContext& renderContext) override;

        /* ----- Pipeline States ----- */

        void SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline) override;
        void SetComputePipeline(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        void BeginQuery(Query& query) override;
        void EndQuery(Query& query) override;

        bool QueryResult(Query& query, std::uint64_t& result) override;

        void BeginRenderCondition(Query& query, const RenderConditionMode mode) override;
        void EndRenderCondition() override;

        /* ----- Drawing ----- */

        void Draw(unsigned int numVertices, unsigned int firstVertex) override;

        void DrawIndexed(unsigned int numVertices, unsigned int firstIndex) override;
        void DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset) override;

        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances) override;
        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset) override;

        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex) override;
        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset) override;
        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset) override;

        /* ----- Compute ----- */

        void Dispatch(unsigned int groupSizeX, unsigned int groupSizeY, unsigned int groupSizeZ) override;

        /* ----- Misc ----- */

        void SyncGPU() override;

    private:

        struct D3D11FramebufferView
        {
            std::vector<ID3D11RenderTargetView*>    rtvList;
            ID3D11DepthStencilView*                 dsv = nullptr;
        };

        void SubmitFramebufferView();

        void SetConstantBuffersOnStages(UINT startSlot, UINT count, ID3D11Buffer* const* buffers, long shaderStageFlags);
        void SetShaderResourcesOnStages(UINT startSlot, UINT count, ID3D11ShaderResourceView* const* views, long shaderStageFlags);
        void SetSamplersOnStages(UINT startSlot, UINT count, ID3D11SamplerState* const* samplers, long shaderStageFlags);
        void SetUnorderedAccessViewsOnStages(UINT startSlot, UINT count, ID3D11UnorderedAccessView* const* views, const UINT* initialCounts, long shaderStageFlags);

        void ResolveBoundRenderTarget();

        D3D11StateManager&          stateMngr_;
        
        ComPtr<ID3D11DeviceContext> context_;

        D3D11FramebufferView        framebufferView_;

        D3DClearState               clearState_;

        D3D11RenderTarget*          boundRenderTarget_  = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
