/*
 * CsRenderSystemChilds.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderSystemChilds.h"


namespace LHermanns
{

namespace LLGL
{


/*
 * Resource class
 */

Resource::Resource(::LLGL::Resource* native) :
    native_ { native }
{
}

::LLGL::Resource* Resource::Native::get()
{
    return native_;
}

LHermanns::LLGL::ResourceType Resource::ResourceType::get()
{
    return static_cast<LHermanns::LLGL::ResourceType>(native_->QueryResourceType());
}


/*
 * Buffer class
 */

Buffer::Buffer(::LLGL::Buffer* native) :
    Resource { native }
{
}

::LLGL::Buffer* Buffer::NativeSub::get()
{
    return static_cast<::LLGL::Buffer*>(Native);
}


/*
 * Texture class
 */

Texture::Texture(::LLGL::Texture* native) :
    Resource { native }
{
}

::LLGL::Texture* Texture::NativeSub::get()
{
    return static_cast<::LLGL::Texture*>(Native);
}

TextureType Texture::Type::get()
{
    return static_cast<LHermanns::LLGL::TextureType>(reinterpret_cast<::LLGL::Texture*>(Native::get())->GetType());
}


/*
 * Sampler class
 */

Sampler::Sampler(::LLGL::Sampler* native) :
    Resource { native }
{
}

::LLGL::Sampler* Sampler::NativeSub::get()
{
    return static_cast<::LLGL::Sampler*>(Native);
}


/*
 * Query class
 */

Query::Query(::LLGL::Query* native) :
    native_ { native }
{
}

::LLGL::Query* Query::Native::get()
{
    return native_;
}

QueryType Query::Type::get()
{
    return static_cast<LHermanns::LLGL::QueryType>(native_->GetType());
}


/*
 * Fence class
 */

Fence::Fence(::LLGL::Fence* native) :
    native_ { native }
{
}

::LLGL::Fence* Fence::Native::get()
{
    return native_;
}


/*
 * RenderPass class
 */

RenderPass::RenderPass(::LLGL::RenderPass* native) :
    native_ { native }
{
}

::LLGL::RenderPass* RenderPass::Native::get()
{
    return native_;
};


/*
 * PipelineLayout class
 */

PipelineLayout::PipelineLayout(::LLGL::PipelineLayout* native) :
    native_ { native }
{
}

::LLGL::PipelineLayout* PipelineLayout::Native::get()
{
    return native_;
};


/*
 * GraphicsPipeline class
 */

GraphicsPipeline::GraphicsPipeline(::LLGL::GraphicsPipeline* native) :
    native_ { native }
{
}

::LLGL::GraphicsPipeline* GraphicsPipeline::Native::get()
{
    return native_;
};


/*
 * ComputePipeline class
 */

ComputePipeline::ComputePipeline(::LLGL::ComputePipeline* native) :
    native_ { native }
{
}

::LLGL::ComputePipeline* ComputePipeline::Native::get()
{
    return native_;
};


/*
 * ResourceHeap class
 */

ResourceHeap::ResourceHeap(::LLGL::ResourceHeap* native) :
    native_ { native }
{
}

::LLGL::ResourceHeap* ResourceHeap::Native::get()
{
    return native_;
};


/*
 * BufferArray class
 */

BufferArray::BufferArray(::LLGL::BufferArray* native) :
    native_ { native }
{
}

::LLGL::BufferArray* BufferArray::Native::get()
{
    return native_;
};


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
