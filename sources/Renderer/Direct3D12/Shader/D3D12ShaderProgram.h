/*
 * D3D12ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_SHADER_PROGRAM_H__
#define __LLGL_D3D12_SHADER_PROGRAM_H__


#include <LLGL/ShaderProgram.h>
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

        bool LinkShaders() override;

        std::string QueryInfoLog() override;

        std::vector<VertexAttribute> QueryVertexAttributes() const override;
        std::vector<ConstantBufferDescriptor> QueryConstantBuffers() const override;
        std::vector<UniformDescriptor> QueryUniforms() const override;

        void BindVertexAttributes(const std::vector<VertexAttribute>& vertexAttribs) override;
        void BindConstantBuffer(const std::string& name, unsigned int bindingIndex) override;
        void BindStorageBuffer(const std::string& name, unsigned int bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

        D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc() const;

        inline D3D12Shader* GetVS() const { return vs_; }
        inline D3D12Shader* GetPS() const { return ps_; }
        inline D3D12Shader* GetDS() const { return ds_; }
        inline D3D12Shader* GetHS() const { return hs_; }
        inline D3D12Shader* GetGS() const { return gs_; }
        inline D3D12Shader* GetCS() const { return cs_; }

    private:

        std::vector<D3D12_INPUT_ELEMENT_DESC>   inputElements_;

        D3D12Shader*                            vs_             = nullptr;
        D3D12Shader*                            ps_             = nullptr;
        D3D12Shader*                            ds_             = nullptr;
        D3D12Shader*                            hs_             = nullptr;
        D3D12Shader*                            gs_             = nullptr;
        D3D12Shader*                            cs_             = nullptr;

        std::vector<VertexAttribute>            vertexAttributes_;
        std::vector<ConstantBufferDescriptor>   constantBufferDescs_;

};


} // /namespace LLGL


#endif



// ================================================================================
