/*
 * D3D11Shader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_SHADER_H
#define LLGL_D3D11_SHADER_H


#include <LLGL/Shader.h>
#include <LLGL/ShaderProgramFlags.h>
#include <LLGL/VertexAttribute.h>
#include <LLGL/BufferFlags.h>
#include "../../DXCommon/ComPtr.h"
#include <vector>
#include <string>
#include <d3d11.h>


namespace LLGL
{


// Native objects of D3D11 shaders.
union D3D11NativeShader
{
    inline D3D11NativeShader() :
        vs { nullptr }
    {
    }
    inline ~D3D11NativeShader()
    {
    }

    ComPtr<ID3D11VertexShader>      vs;
    ComPtr<ID3D11HullShader>        hs;
    ComPtr<ID3D11DomainShader>      ds;
    ComPtr<ID3D11GeometryShader>    gs;
    ComPtr<ID3D11PixelShader>       ps;
    ComPtr<ID3D11ComputeShader>     cs;
};


class D3D11Shader final : public Shader
{

    public:

        D3D11Shader(ID3D11Device* device, const ShaderDescriptor& desc);

        bool HasErrors() const override;

        std::string Disassemble(int flags = 0) override;

        std::string QueryInfoLog() override;

        /* ----- Extended internal functions ---- */

        void Reflect(ShaderReflectionDescriptor& reflectionDesc) const;

        // Returns the native D3D shader object.
        inline const D3D11NativeShader& GetNative() const
        {
            return native_;
        }

        // Returns the shader byte code container.
        inline const std::vector<char>& GetByteCode() const
        {
            return byteCode_;
        }

    private:

        bool Build(ID3D11Device* device, const ShaderDescriptor& shaderDesc);
        bool CompileSource(ID3D11Device* device, const ShaderDescriptor& shaderDesc);
        bool LoadBinary(ID3D11Device* device, const ShaderDescriptor& shaderDesc);

        void CreateNativeShader(ID3D11Device* device, const ShaderDescriptor::StreamOutput& streamOutputDesc, ID3D11ClassLinkage* classLinkage);

        void ReflectShaderByteCode(ShaderReflectionDescriptor& reflectionDesc) const;

        D3D11NativeShader   native_;

        std::vector<char>   byteCode_;
        ComPtr<ID3DBlob>    errors_;
        bool                hasErrors_  = false;

};


} // /namespace LLGL


#endif



// ================================================================================
