/*
 * WGShaderModulePool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_SHADER_MODULE_POOL_H
#define LLGL_WG_SHADER_MODULE_POOL_H


#include "WGShaderModule.h"
#include <vector>


namespace LLGL
{


// Singleton pool for WebGPU shader modules.
class WGShaderModulePool
{

    public:

        WGShaderModulePool(const WGShaderModulePool&) = delete;
        WGShaderModulePool& operator = (const WGShaderModulePool&) = delete;

        // Returns the instance of this pool.
        static WGShaderModulePool& Get();

        // Clear all resource containers of this pool (used by WGRenderSystem).
        void Clear();

        WGShaderModuleSPtr CreateShaderModule(WGPUInstance instance, WGPUDevice device, const ShaderSourceContext& sourceContext);
        void ReleaseShaderModule(WGShaderModuleSPtr&& shaderModule);

    private:

        using SourceBlob = std::vector<char>;

        struct ShaderModuleSourcePair
        {
            SourceBlob          sourceBlob;
            WGShaderModuleSPtr  shaderModule;
        };

    private:

        WGShaderModulePool() = default;

        inline static SourceBlob ToSourceBlob(StringView source)
        {
            return SourceBlob{ source.begin(), source.end() };
        }

        ShaderModuleSourcePair* FindShaderModuleWithSource(const SourceBlob& sourceBlob, std::size_t* outIndex = nullptr);

    private:

        std::vector<ShaderModuleSourcePair> shaderModules_;

};


} // /namespace LLGL


#endif



// ================================================================================
