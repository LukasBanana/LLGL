/*
 * D3D11BuiltinShaderFactory.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_BUILTIN_SHADER_FACTORY_H
#define LLGL_D3D11_BUILTIN_SHADER_FACTORY_H


#include "D3D11Shader.h"


namespace LLGL
{


// Enumeration of all builtin D3D11 shaders.
enum class D3D11BuiltinShader
{
    CopyTexture1DFromBufferCS = 0,
    CopyTexture2DFromBufferCS,
    CopyTexture3DFromBufferCS,
    CopyBufferFromTexture1DCS,
    CopyBufferFromTexture2DCS,
    CopyBufferFromTexture3DCS,
    Num
};

// Builtin D3D11 shader factory singleton.
class D3D11BuiltinShaderFactory
{

    public:

        D3D11BuiltinShaderFactory(const D3D11BuiltinShaderFactory&) = delete;
        D3D11BuiltinShaderFactory& operator = (const D3D11BuiltinShaderFactory&) = delete;

        // Returns the instance of this singleton.
        static D3D11BuiltinShaderFactory& Get();

        // Creats all builtin shaders with the specified D3D device.
        void CreateBuiltinShaders(ID3D11Device* device);

        // Releases all builtin shaders.
        void Clear();

        // Returns the specified native builtin shader.
        ID3D11ComputeShader* GetBulitinComputeShader(const D3D11BuiltinShader builtin) const;

    private:

        D3D11BuiltinShaderFactory() = default;

        void LoadBuiltinShader(ID3D11Device* device,
            const D3D11BuiltinShader builtin,
            const BYTE* shaderBytecode,
            size_t shaderBytecodeSize);

    private:

        static constexpr std::size_t numBuiltinShaders = static_cast<std::size_t>(D3D11BuiltinShader::Num);

        ComPtr<ID3D11ComputeShader> builtinComputeShaders_[D3D11BuiltinShaderFactory::numBuiltinShaders];

};


} // /namespace LLGL


#endif



// ================================================================================
