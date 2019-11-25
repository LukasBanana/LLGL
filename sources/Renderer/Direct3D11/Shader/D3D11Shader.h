/*
 * D3D11Shader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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


// Union for easy handling of native D3D11 shader objects.
union D3D11NativeShader
{
    inline D3D11NativeShader() :
        vs { nullptr }
    {
    }
    inline D3D11NativeShader(const D3D11NativeShader& rhs) :
        vs { rhs.vs }
    {
    }
    inline D3D11NativeShader& operator = (const D3D11NativeShader& rhs)
    {
        vs = rhs.vs;
        return *this;
    }
    inline ~D3D11NativeShader()
    {
        vs.Reset();
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

        void SetName(const char* name) override;

        bool HasErrors() const override;

        std::string GetReport() const override;

    public:

        D3D11Shader(ID3D11Device* device, const ShaderDescriptor& desc);

        bool Reflect(ShaderReflection& reflection) const;

        // Returns the native D3D shader object.
        inline const D3D11NativeShader& GetNative() const
        {
            return native_;
        }

        // Returns the shader byte code container.
        inline ID3DBlob* GetByteCode() const
        {
            return byteCode_.Get();
        }

        // Returns the input layout for vertex shaders.
        inline const ComPtr<ID3D11InputLayout>& GetInputLayout() const
        {
            return inputLayout_;
        }

    public:

        // Creates a native D3D11 shader from the specified byte code blob.
        static D3D11NativeShader CreateNativeShaderFromBlob(
            ID3D11Device*           device,
            const ShaderType        type,
            ID3DBlob*               blob,
            std::size_t             numStreamOutputAttribs  = 0,
            const VertexAttribute*  streamOutputAttribs     = nullptr,
            ID3D11ClassLinkage*     classLinkage            = nullptr
        );

    private:

        bool BuildShader(ID3D11Device* device, const ShaderDescriptor& shaderDesc);
        void BuildInputLayout(ID3D11Device* device, UINT numVertexAttribs, const VertexAttribute* vertexAttribs);

        bool CompileSource(ID3D11Device* device, const ShaderDescriptor& shaderDesc);
        bool LoadBinary(ID3D11Device* device, const ShaderDescriptor& shaderDesc);

        void CreateNativeShader(
            ID3D11Device*           device,
            std::size_t             numStreamOutputAttribs  = 0,
            const VertexAttribute*  streamOutputAttribs     = nullptr,
            ID3D11ClassLinkage*     classLinkage            = nullptr
        );

        HRESULT ReflectShaderByteCode(ShaderReflection& reflection) const;

    private:

        D3D11NativeShader           native_;

        ComPtr<ID3DBlob>            byteCode_;
        ComPtr<ID3DBlob>            errors_;
        bool                        hasErrors_  = false;

        ComPtr<ID3D11InputLayout>   inputLayout_;

};


} // /namespace LLGL


#endif



// ================================================================================
