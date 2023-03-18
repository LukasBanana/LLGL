/*
 * VKPipelineState.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_PIPELINE_STATE_H
#define LLGL_VK_PIPELINE_STATE_H


#include <LLGL/PipelineState.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"


namespace LLGL
{


class PipelineLayout;
class VKPipelineLayout;

class VKPipelineState : public PipelineState
{

    public:

        VKPipelineState(
            const VKPtr<VkDevice>&  device,
            VkPipelineBindPoint     bindPoint,
            const PipelineLayout*   pipelineLayout = nullptr
        );

        const Report* GetReport() const override;

    public:

        // Binds this pipeline state to the specified Vulkan command buffer.
        void BindPipeline(VkCommandBuffer commandBuffer);

        // Returns the native PSO.
        inline VkPipeline GetVkPipeline() const
        {
            return pipeline_.Get();
        }

        // Returns the pipeline binding point.
        inline VkPipelineBindPoint GetBindPoint() const
        {
            return bindPoint_;
        }

        // Returns the pipeline layout this PSO was created with.
        inline const VKPipelineLayout* GetPipelineLayout() const
        {
            return pipelineLayout_;
        }

    protected:

        // Releases the native PSO and returns its address.
        VkPipeline* ReleaseAndGetAddressOfVkPipeline();

        // Returns the native Vulkan pipeline layout this PSO was created with or the specified layout if there was no layout specified.
        VkPipelineLayout GetVkPipelineLayoutOrDefault(VkPipelineLayout defaultPipelineLayout) const;

    private:

        VKPtr<VkPipeline>       pipeline_;
        const VKPipelineLayout* pipelineLayout_ = nullptr;
        VkPipelineBindPoint     bindPoint_      = VK_PIPELINE_BIND_POINT_MAX_ENUM;

};


} // /namespace LLGL


#endif



// ================================================================================
