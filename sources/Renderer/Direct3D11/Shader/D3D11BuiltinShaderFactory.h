/*
 * D3D11BuiltinShaderFactory.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
        const D3D11NativeShader& GetBulitinShader(const D3D11BuiltinShader builtin) const;

    private:

        D3D11BuiltinShaderFactory() = default;

        void LoadBuiltinShader(ID3D11Device* device, const D3D11BuiltinShader builtin, int resourceID);

    private:

        static const std::size_t g_numBuiltinShaders = static_cast<std::size_t>(D3D11BuiltinShader::Num);

        D3D11NativeShader builtinShaders_[D3D11BuiltinShaderFactory::g_numBuiltinShaders];

};


} // /namespace LLGL


#endif



// ================================================================================
