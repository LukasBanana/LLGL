/*
 * D3D12Shader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_SHADER_H
#define LLGL_D3D12_SHADER_H


#include <LLGL/Shader.h>
#include <LLGL/ShaderReflection.h>
#include <LLGL/VertexAttribute.h>
#include <LLGL/BufferFlags.h>
#include <LLGL/Report.h>
#include "../../DXCommon/ComPtr.h"
#include "../../../Core/LinearStringContainer.h"
#include <vector>
#include <d3d12.h>


namespace LLGL
{


struct D3D12ConstantReflection
{
    std::string name;   // Name of the constant buffer field.
    UINT        offset; // Offset (in bytes) within the constant buffer the uniform's root parameter occupies.
    UINT        size;   // Size (in bytes) of this uniform.
};

struct D3D12ConstantBufferReflection
{
    long                                    stageFlags;
    D3D12_ROOT_CONSTANTS                    rootConstants;
    std::vector<D3D12ConstantReflection>    fields;
};

class D3D12RenderSystem;

class D3D12Shader final : public Shader
{

    public:

        #include <LLGL/Backend/Shader.inl>

    public:

        D3D12Shader(D3D12RenderSystem& renderSystem, const ShaderDescriptor& desc);

    public:

        D3D12_SHADER_BYTECODE GetByteCode() const;

        bool GetInputLayoutDesc(D3D12_INPUT_LAYOUT_DESC& layoutDesc) const;
        bool GetStreamOutputDesc(D3D12_STREAM_OUTPUT_DESC& layoutDesc) const;

        // Returns a list of all reflected constant buffers including their fields.
        HRESULT ReflectAndCacheConstantBuffers(const std::vector<D3D12ConstantBufferReflection>** outConstantBuffers);

    private:

        bool BuildShader(const ShaderDescriptor& shaderDesc);
        void ReserveVertexAttribs(const ShaderDescriptor& shaderDesc);
        void BuildInputLayout(UINT numVertexAttribs, const VertexAttribute* vertexAttribs);
        void BuildStreamOutput(UINT numVertexAttribs, const VertexAttribute* vertexAttribs);

        bool CompileSource(const ShaderDescriptor& shaderDesc);
        bool LoadBinary(const ShaderDescriptor& shaderDesc);

        HRESULT ReflectShaderByteCode(ShaderReflection& reflection) const;

        HRESULT ReflectConstantBuffers(std::vector<D3D12ConstantBufferReflection>& outConstantBuffers) const;

    private:

		D3D12RenderSystem&    						renderSystem_;

        ComPtr<ID3DBlob>                            byteCode_;
        Report                                      report_;

        std::vector<D3D12_INPUT_ELEMENT_DESC>       inputElements_;
        std::vector<D3D12_SO_DECLARATION_ENTRY>     soDeclEntries_;
        std::vector<UINT>                           soBufferStrides_;
        LinearStringContainer                       vertexAttribNames_; // custom string container to hold valid string pointers.

        HRESULT                                     cbufferReflectionResult_    = S_FALSE;
        std::vector<D3D12ConstantBufferReflection>  cbufferReflections_;

};


} // /namespace LLGL


#endif



// ================================================================================
