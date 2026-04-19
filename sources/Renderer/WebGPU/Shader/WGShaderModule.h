/*
 * WGShaderModule.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_SHADER_MODULE_H
#define LLGL_WG_SHADER_MODULE_H


#include <LLGL/ShaderFlags.h>
#include <LLGL/Report.h>
#include "WGResourceReflectionTable.h"
#include "../WGPtr.h"
#include <memory>
#include <webgpu/webgpu.h>


namespace LLGL
{


struct ShaderSourceContext;

// WebGPU shader module class. These are shared across one or more WGShader object,
// since WGShader is just a wrapper around these modules with the addition of the entry point name.
class WGShaderModule
{

    public:

        WGShaderModule(WGPUInstance instance, WGPUDevice device, const ShaderSourceContext& sourceContext);

        inline const Report* GetReport() const
        {
            return (report_ ? &report_ : nullptr);
        }

        // Returns the native WebGPU shader module.
        inline WGPUShaderModule GetNative() const
        {
            return shaderModule_;
        }

        // Returns the WGSL resource reflection.
        inline const WGResourceReflectionTable& GetResourceReflectionTable() const
        {
            return resourceReflectionTable_;
        }

    private:

        void BuildShader(WGPUInstance instance, WGPUDevice device, const ShaderSourceContext& sourceContext);

    private:

        WGPtr<WGPUShaderModule>     shaderModule_;
        WGResourceReflectionTable   resourceReflectionTable_;
        Report                      report_;

};

using WGShaderModuleSPtr = std::shared_ptr<WGShaderModule>;


} // /namespace LLGL


#endif



// ================================================================================
