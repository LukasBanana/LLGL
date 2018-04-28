/*
 * D3D12ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_SHADER_PROGRAM_H
#define LLGL_D3D12_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include "../../DXCommon/ComPtr.h"
#include <vector>
#include <d3d12.h>


namespace LLGL
{


class D3D12Shader;

class D3D12ShaderProgram : public ShaderProgram
{

    public:

        D3D12ShaderProgram();

        void AttachShader(Shader& shader) override;
        void DetachAll() override;

        bool LinkShaders() override;

        std::string QueryInfoLog() override;

        std::vector<VertexAttribute> QueryVertexAttributes() const override;
        std::vector<StreamOutputAttribute> QueryStreamOutputAttributes() const override;
        std::vector<ConstantBufferViewDescriptor> QueryConstantBuffers() const override;
        std::vector<StorageBufferViewDescriptor> QueryStorageBuffers() const override;
        std::vector<UniformDescriptor> QueryUniforms() const override;

        void BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats) override;
        void BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex) override;
        void BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

        /* ----- Extended internal functions ----- */

        D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc() const;

        inline D3D12Shader* GetVS() const { return vs_; }
        inline D3D12Shader* GetPS() const { return ps_; }
        inline D3D12Shader* GetHS() const { return hs_; }
        inline D3D12Shader* GetDS() const { return ds_; }
        inline D3D12Shader* GetGS() const { return gs_; }
        inline D3D12Shader* GetCS() const { return cs_; }

        inline UINT GetNumSRV() const
        {
            return numSRV_;
        }

        inline UINT GetNumCBV() const
        {
            return static_cast<UINT>(constantBufferDescs_.size());
        }

        inline UINT GetNumUAV() const
        {
            return static_cast<UINT>(storageBufferDescs_.size());
        }

    private:

        std::vector<D3D12_INPUT_ELEMENT_DESC>       inputElements_;

        D3D12Shader*                                vs_                     = nullptr;
        D3D12Shader*                                hs_                     = nullptr;
        D3D12Shader*                                ds_                     = nullptr;
        D3D12Shader*                                gs_                     = nullptr;
        D3D12Shader*                                ps_                     = nullptr;
        D3D12Shader*                                cs_                     = nullptr;

        std::vector<VertexAttribute>                vertexAttributes_;
        std::vector<ConstantBufferViewDescriptor>   constantBufferDescs_;
        std::vector<StorageBufferViewDescriptor>    storageBufferDescs_;

        LinkError                                   linkError_              = LinkError::NoError;

        UINT                                        numSRV_                 = 0;//TODO: use "TextureViewDescriptor" or the like

};


} // /namespace LLGL


#endif



// ================================================================================
