/*
 * D3D9Shader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_SHADER_H
#define LLGL_D3D9_SHADER_H


#include <LLGL/Shader.h>
#include "../Direct3D9.h"
#include "../../DXCommon/ComPtr.h"
#include <d3dcommon.h>
#include <string>
#include <memory>
#include <vector>
#include <cstdint>


namespace LLGL
{


enum class D3D9UniformType
{
    Undefined,

    Bool,
    Int,
    Float,
};

struct D3D9ShaderRegister
{
    std::uint16_t index     : 14;
    std::uint16_t component :  2;
    std::uint16_t count;
};

struct D3D9ShaderConstant
{
    std::string         name;
    D3D9UniformType     type        = D3D9UniformType::Undefined;
    D3D9ShaderRegister  reg         = {};
    std::uint16_t       rows        = 0;
    std::uint16_t       columns     = 0;
    std::uint16_t       arraySize   = 0;
    std::uint32_t       byteOffset  = 0;
    std::uint32_t       byteSize    = 0;
};

struct D3D9ShaderConstantTable
{
    std::vector<D3D9ShaderConstant> constants;
    std::uint32_t                   byteSize    = 0;
};

// Base class for D3D9VertexShader and D3D9PixelShader.
class D3D9Shader : public Shader
{

    public:

        void SetDebugName(const char* name) override final;
        const Report* GetReport() const override final;

        inline const D3D9ShaderConstantTable& GetConstantTable() const
        {
            return constantTable_;
        }

    protected:

        D3D9Shader(const ShaderType type);

        bool BuildShader(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc);

    protected:

        virtual HRESULT CreateD3DShaderFromBlob(IDirect3DDevice9* device, ID3DBlob* byteCode) = 0;

    private:

        bool CompileSource(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc);
        bool LoadBinary(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc);

        bool ReflectConstantTable(ID3DBlob* byteCode);

    private:

        Report                  report_;
        D3D9ShaderConstantTable constantTable_;

};


} // /namespace LLGL


#endif



// ================================================================================
