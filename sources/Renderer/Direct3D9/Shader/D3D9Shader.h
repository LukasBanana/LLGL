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
#include <d3dcommon.h>


namespace LLGL
{


class D3D9Shader : public Shader
{

    public:

        void SetDebugName(const char* name) override final;
        const Report* GetReport() const override final;

    protected:

        D3D9Shader(const ShaderType type);

        bool BuildShader(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc);

    protected:

        virtual HRESULT CreateD3DShaderFromBlob(IDirect3DDevice9* device, ID3DBlob* byteCode) = 0;

    private:

        bool CompileSource(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc);
        bool LoadBinary(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc);

        HRESULT ReflectShaderByteCode(ID3DBlob* byteCode);

    private:

        Report report_;

};


} // /namespace LLGL


#endif



// ================================================================================
