/*
 * PipelineLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_PIPELINE_LAYOUT_H
#define LLGL_PIPELINE_LAYOUT_H


#include <LLGL/RenderSystemChild.h>
#include <cstdint>


namespace LLGL
{


/**
\brief Pipeline layout interface.
\remarks An instance of this interface provides a layout for resource binding in a graphics or compute pipeline.
\see RenderSystem::CreatePipelineLayout
\see GraphicsPipelineDescriptor::pipelineLayout
\see ResourceHeapDescriptor::pipelineLayout
*/
class LLGL_EXPORT PipelineLayout : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::PipelineLayout );

    public:

        /**
        \brief Returns the number of resource view heap bindings in this pipeline layout.
        \remarks This only includes the resource bindings for a ResourceHeap, i.e. PipelineLayoutDescriptor::heapBindings.
        \see PipelineLayoutDescriptor::heapBindings
        \see ResourceHeap::GetNumDescriptorSets
        */
        virtual std::uint32_t GetNumHeapBindings() const = 0;

        /**
        \brief Returns the number of resource bindings in this pipeline layout.
        \remarks This does \e not include the resource bindings for a ResourceHeap, i.e. PipelineLayoutDescriptor::heapBindings.
        \see PipelineLayoutDescriptor::bindings
        \see CommandBuffer::SetResource
        */
        virtual std::uint32_t GetNumBindings() const = 0;

        /**
        \brief Returns the number of static sampler states in this pipeline layout.
        \see PipelineLayoutDescriptor::staticSamplers
        */
        virtual std::uint32_t GetNumStaticSamplers() const = 0;

        /**
        \brief Returns the number of uniforms in this pipeline layout.
        \see PipelineLayoutDescriptor::uniforms
        */
        virtual std::uint32_t GetNumUniforms() const = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
