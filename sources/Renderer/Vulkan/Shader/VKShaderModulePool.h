/*
 * VKShaderModulePool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_SHADER_MODULE_POOL_H
#define LLGL_VK_SHADER_MODULE_POOL_H


#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>


namespace LLGL
{


class VKShader;
class VKPipelineLayout;

// Singleton pool for Vulkan shader/pipeline-layout permutations.
class VKShaderModulePool
{

    public:

        VKShaderModulePool(const VKShaderModulePool&) = delete;
        VKShaderModulePool& operator = (const VKShaderModulePool&) = delete;

        // Returns the instance of this pool.
        static VKShaderModulePool& Get();

        // Clear all resource containers of this pool (used by VKRenderSystem).
        void Clear();

        /* ----- Depth-stencil states ----- */

        VkShaderModule GetOrCreateVkShaderModulePermutation(VKShader& shader, const VKPipelineLayout& pipelineLayout);

        void NotifyReleaseShader(VKShader* shader);
        void NotifyReleasePipelineLayout(VKPipelineLayout* pipelineLayout);

    private:

        struct ShaderModulePermutation
        {
            const VKPipelineLayout* pipelineLayout  = nullptr;
            const VKShader*         shader          = nullptr;
            VKPtr<VkShaderModule>   shaderModule;
        };

    private:

        VKShaderModulePool() = default;

    private:

        std::vector<ShaderModulePermutation> permutations_;

};


} // /namespace LLGL


#endif



// ================================================================================
