/*
 * D3D11Shader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_SHADER_H
#define LLGL_D3D11_SHADER_H


#include <LLGL/Shader.h>
#include <LLGL/VertexAttribute.h>
#include <LLGL/BufferFlags.h>
#include "../../DXCommon/ComPtr.h"
#include <vector>
#include <string>
#include <d3d11.h>


namespace LLGL
{


union D3D11NativeShader
{
    D3D11NativeShader() :
        vs { nullptr }
    {
    }
    ~D3D11NativeShader()
    {
    }

    ComPtr<ID3D11VertexShader>      vs;
    ComPtr<ID3D11HullShader>        hs;
    ComPtr<ID3D11DomainShader>      ds;
    ComPtr<ID3D11GeometryShader>    gs;
    ComPtr<ID3D11PixelShader>       ps;
    ComPtr<ID3D11ComputeShader>     cs;
};


class D3D11Shader : public Shader
{

    public:

        D3D11Shader(ID3D11Device* device, const ShaderType type);

        bool Compile(const std::string& sourceCode, const ShaderDescriptor& shaderDesc = {}) override;

        bool LoadBinary(std::vector<char>&& binaryCode, const ShaderDescriptor& shaderDesc = {}) override;

        std::string Disassemble(int flags = 0) override;

        std::string QueryInfoLog() override;

        /* ----- Extended internal functions ---- */

        void Reflect(ShaderReflectionDescriptor& reflectionDesc) const;

        inline const D3D11NativeShader& GetNativeShader() const
        {
            return nativeShader_;
        }

        inline const std::vector<char>& GetByteCode() const
        {
            return byteCode_;
        }

    private:

        void CreateNativeShader(const ShaderDescriptor::StreamOutput& streamOutputDesc, ID3D11ClassLinkage* classLinkage);

        void ReflectShaderByteCode(ShaderReflectionDescriptor& reflectionDesc) const;

        ID3D11Device*       device_         = nullptr;

        D3D11NativeShader   nativeShader_;

        std::vector<char>   byteCode_;
        ComPtr<ID3DBlob>    errors_;

};


} // /namespace LLGL


#endif



// ================================================================================
