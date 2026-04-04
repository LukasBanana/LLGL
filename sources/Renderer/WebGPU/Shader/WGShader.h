/*
 * WGShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_SHADER_H
#define LLGL_WG_SHADER_H


#include <LLGL/Shader.h>
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGShader final : public Shader
{

    public:

        #include <LLGL/Backend/Shader.inl>

    public:

        WGShader(const ShaderDescriptor& desc);

        // Returns the native WebGPU shader module.
        inline WGPUShaderModule GetNative() const
        {
            return shaderModule_;
        }

    private:

        WGPUShaderModule shaderModule_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
