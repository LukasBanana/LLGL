/*
 * D3D12Shader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_SHADER_H
#define LLGL_D3D12_SHADER_H


#include <LLGL/Shader.h>
#include <LLGL/ShaderProgramFlags.h>
#include <LLGL/VertexAttribute.h>
#include <LLGL/BufferFlags.h>
#include "../../DXCommon/ComPtr.h"
#include <vector>
#include <d3d12.h>


namespace LLGL
{


class D3D12Shader final : public Shader
{

    public:

        D3D12Shader(const ShaderDescriptor& desc);

        bool HasErrors() const override;

        std::string Disassemble(int flags = 0) override;

        std::string QueryInfoLog() override;

    public:

        D3D12_SHADER_BYTECODE GetByteCode() const;

        void Reflect(ShaderReflectionDescriptor& reflectionDesc) const;

    private:

        bool Build(const ShaderDescriptor& shaderDesc);
        bool CompileSource(const ShaderDescriptor& shaderDesc);
        bool LoadBinary(const ShaderDescriptor& shaderDesc);

        void ReflectShaderByteCode(ShaderReflectionDescriptor& reflectionDesc) const;

    private:

        std::vector<char>   byteCode_;
        ComPtr<ID3DBlob>    errors_;
        bool                hasErrors_  = false;

};


} // /namespace LLGL


#endif



// ================================================================================
