/*
 * WGShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_SHADER_H
#define LLGL_WG_SHADER_H


#include <LLGL/Shader.h>
#include <LLGL/ShaderFlags.h>
#include "WGShaderModule.h"
#include "WGVertexInputLayout.h"
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGShader final : public Shader
{

    public:

        #include <LLGL/Backend/Shader.inl>

    public:

        WGShader(WGPUInstance instance, WGPUDevice device, const ShaderDescriptor& desc);
        ~WGShader();

        // Returns the native WebGPU shader module.
        inline WGPUShaderModule GetNative() const
        {
            return shaderModule_->GetNative();
        }

        // Returns a WebGPU string view of the entry point name for this shader.
        // This is used when the shared shader module is bound to a render or compute pipeline.
        inline WGPUStringView GetEntryPointNameView() const
        {
            return WGPUStringView{ entryPoint_.data(), entryPoint_.size() };
        }

        // Returns the vertex input layout or null if there is none.
        inline const WGVertexInputLayout* GetVertexInputLayout() const
        {
            return vertexInputLayout_.get();
        }

    private:

        void BuildShader(WGPUInstance instance, WGPUDevice device, const ShaderDescriptor& shaderDesc);

    private:

        WGShaderModuleSPtr      shaderModule_;
        WGVertexInputLayoutPtr  vertexInputLayout_;
        std::string             entryPoint_;
        Report                  report_;

};


} // /namespace LLGL


#endif



// ================================================================================
