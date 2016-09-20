/*
 * D3D11ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_SHADER_PROGRAM_H__
#define __LLGL_D3D11_SHADER_PROGRAM_H__


#include <LLGL/ShaderProgram.h>
#include "../../ComPtr.h"
#include <vector>
#include <d3d11.h>


namespace LLGL
{


class D3D11Shader;

class D3D11ShaderProgram : public ShaderProgram
{

    public:

        D3D11ShaderProgram();

        void AttachShader(Shader& shader) override;

        bool LinkShaders() override;

        std::string QueryInfoLog() override;

        std::vector<VertexAttribute> QueryVertexAttributes() const override;
        std::vector<ConstantBufferDescriptor> QueryConstantBuffers() const override;
        std::vector<StorageBufferDescriptor> QueryStorageBuffers() const override;
        std::vector<UniformDescriptor> QueryUniforms() const override;

        void BindVertexAttributes(const std::vector<VertexAttribute>& vertexAttribs) override;
        void BindConstantBuffer(const std::string& name, unsigned int bindingIndex) override;
        void BindStorageBuffer(const std::string& name, unsigned int bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

        /* ----- Extended internal functions ----- */

        inline const std::vector<D3D11_INPUT_ELEMENT_DESC>& GetInputElements() const
        {
            return inputElements_;
        }

        inline D3D11Shader* GetVS() const { return vs_; }
        inline D3D11Shader* GetPS() const { return ps_; }
        inline D3D11Shader* GetDS() const { return ds_; }
        inline D3D11Shader* GetHS() const { return hs_; }
        inline D3D11Shader* GetGS() const { return gs_; }
        inline D3D11Shader* GetCS() const { return cs_; }

    private:

        enum class LinkError
        {
            NoError,
            Composition,
            ByteCode,
        };

        std::vector<D3D11_INPUT_ELEMENT_DESC>   inputElements_;

        D3D11Shader*                            vs_                     = nullptr;
        D3D11Shader*                            ps_                     = nullptr;
        D3D11Shader*                            ds_                     = nullptr;
        D3D11Shader*                            hs_                     = nullptr;
        D3D11Shader*                            gs_                     = nullptr;
        D3D11Shader*                            cs_                     = nullptr;

        std::vector<VertexAttribute>            vertexAttributes_;
        std::vector<ConstantBufferDescriptor>   constantBufferDescs_;
        std::vector<StorageBufferDescriptor>    storageBufferDescs_;

        LinkError                               linkError_              = LinkError::NoError;

};


} // /namespace LLGL


#endif



// ================================================================================
