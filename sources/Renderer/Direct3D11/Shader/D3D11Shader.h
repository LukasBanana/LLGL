/*
 * D3D11Shader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_SHADER_H
#define LLGL_D3D11_SHADER_H


#include <LLGL/Shader.h>
#include <LLGL/ShaderReflection.h>
#include <LLGL/VertexAttribute.h>
#include <LLGL/BufferFlags.h>
#include <LLGL/Report.h>
#include "../../DXCommon/ComPtr.h"
#include <vector>
#include <string>
#include <d3d11.h>


namespace LLGL
{


struct D3D11ConstantReflection
{
    std::string name;   // Name of the constant buffer field.
    UINT        offset; // Offset (in bytes) within the constant buffer.
    UINT        size;   // Size (in bytes) of this uniform.
};

struct D3D11ConstantBufferReflection
{
    UINT                                    slot;
    UINT                                    size;
    std::vector<D3D11ConstantReflection>    fields;
};

class D3D11Shader : public Shader
{

    public:

        #include <LLGL/Backend/Shader.inl>

    public:

        void SetDebugName(const char* name) override final;

    public:

        D3D11Shader(const ShaderType type);

        // Returns a list of all reflected constant buffers including their fields.
        HRESULT ReflectAndCacheConstantBuffers(const std::vector<D3D11ConstantBufferReflection>** outConstantBuffers);

        // Returns the native D3D shader object.
        inline const ComPtr<ID3D11DeviceChild>& GetNative() const
        {
            return native_;
        }

        // Returns the shader byte code container.
        inline ID3DBlob* GetByteCode() const
        {
            return byteCode_.Get();
        }

    public:

        // Creates a native D3D11 shader from the specified byte code blob.
        static ComPtr<ID3D11DeviceChild> CreateNativeShaderFromBlob(
            ID3D11Device*           device,
            const ShaderType        type,
            ID3DBlob*               blob,
            std::size_t             numStreamOutputAttribs  = 0,
            const VertexAttribute*  streamOutputAttribs     = nullptr,
            UINT                    rasterizedStream        = D3D11_SO_NO_RASTERIZED_STREAM,
            ID3D11ClassLinkage*     classLinkage            = nullptr
        );

    protected:

        bool BuildShader(ID3D11Device* device, const ShaderDescriptor& shaderDesc);
        bool BuildProxyGeometryShader(ID3D11Device* device, const ShaderDescriptor& shaderDesc, ComPtr<ID3D11GeometryShader>& outProxyGeomtryShader);

    private:

        bool CompileSource(ID3D11Device* device, const ShaderDescriptor& shaderDesc);
        bool LoadBinary(ID3D11Device* device, const ShaderDescriptor& shaderDesc);

        void CreateNativeShader(
            ID3D11Device*           device,
            std::size_t             numStreamOutputAttribs  = 0,
            const VertexAttribute*  streamOutputAttribs     = nullptr
        );

        HRESULT ReflectShaderByteCode(ShaderReflection& reflection) const;

        HRESULT ReflectConstantBuffers(std::vector<D3D11ConstantBufferReflection>& outConstantBuffers) const;

    private:

        ComPtr<ID3D11DeviceChild>                   native_;

        ComPtr<ID3DBlob>                            byteCode_;
        Report                                      report_;

        HRESULT                                     cbufferReflectionResult_    = S_FALSE;
        std::vector<D3D11ConstantBufferReflection>  cbufferReflections_;

};


} // /namespace LLGL


#endif



// ================================================================================
