/*
 * D3D12ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_SHADER_PROGRAM_H__
#define __LLGL_D3D12_SHADER_PROGRAM_H__


#include <LLGL/ShaderProgram.h>
#include <d3d12.h>


namespace LLGL
{


class D3D12Shader;

class D3D12ShaderProgram : public ShaderProgram
{

    public:

        D3D12ShaderProgram();
        ~D3D12ShaderProgram();

        void AttachShader(Shader& shader) override;

        bool LinkShaders() override;

        std::string QueryInfoLog() override;

        std::vector<VertexAttribute> QueryVertexAttributes() const override;
        std::vector<ConstantBufferDescriptor> QueryConstantBuffers() const override;
        std::vector<UniformDescriptor> QueryUniforms() const override;

        void BindVertexAttributes(const std::vector<VertexAttribute>& vertexAttribs) override;
        void BindConstantBuffer(const std::string& name, unsigned int bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

};


} // /namespace LLGL


#endif



// ================================================================================
