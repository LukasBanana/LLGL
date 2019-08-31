/*
 * CsRenderSystemChilds.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderSystemChilds.h"


namespace SharpLLGL
{


/*
 * Resource class
 */

Resource::Resource(LLGL::Resource* native) :
    native_ { native }
{
}

LLGL::Resource* Resource::Native::get()
{
    return native_;
}

SharpLLGL::ResourceType Resource::ResourceType::get()
{
    return static_cast<SharpLLGL::ResourceType>(native_->GetResourceType());
}


/*
 * Buffer class
 */

Buffer::Buffer(LLGL::Buffer* native) :
    Resource { native }
{
}

LLGL::Buffer* Buffer::NativeSub::get()
{
    return static_cast<LLGL::Buffer*>(Native);
}

BindFlags Buffer::BindFlags::get()
{
    return static_cast<SharpLLGL::BindFlags>(reinterpret_cast<LLGL::Buffer*>(Native)->GetBindFlags());
}


/*
 * Texture class
 */

Texture::Texture(LLGL::Texture* native) :
    Resource { native }
{
}

LLGL::Texture* Texture::NativeSub::get()
{
    return static_cast<LLGL::Texture*>(Native);
}

TextureType Texture::Type::get()
{
    return static_cast<TextureType>(reinterpret_cast<LLGL::Texture*>(Native)->GetType());
}


/*
 * Sampler class
 */

Sampler::Sampler(LLGL::Sampler* native) :
    Resource { native }
{
}

LLGL::Sampler* Sampler::NativeSub::get()
{
    return static_cast<LLGL::Sampler*>(Native);
}


/*
 * QueryHeap class
 */

QueryHeap::QueryHeap(LLGL::QueryHeap* native) :
    native_ { native }
{
}

LLGL::QueryHeap* QueryHeap::Native::get()
{
    return native_;
}

QueryType QueryHeap::Type::get()
{
    return static_cast<QueryType>(native_->GetType());
}


/*
 * Fence class
 */

Fence::Fence(LLGL::Fence* native) :
    native_ { native }
{
}

LLGL::Fence* Fence::Native::get()
{
    return native_;
}


/*
 * RenderPass class
 */

RenderPass::RenderPass(LLGL::RenderPass* native) :
    native_ { native }
{
}

LLGL::RenderPass* RenderPass::Native::get()
{
    return native_;
};


/*
 * PipelineLayout class
 */

PipelineLayout::PipelineLayout(LLGL::PipelineLayout* native) :
    native_ { native }
{
}

LLGL::PipelineLayout* PipelineLayout::Native::get()
{
    return native_;
};


/*
 * GraphicsPipeline class
 */

GraphicsPipeline::GraphicsPipeline(LLGL::GraphicsPipeline* native) :
    native_ { native }
{
}

LLGL::GraphicsPipeline* GraphicsPipeline::Native::get()
{
    return native_;
};


/*
 * ComputePipeline class
 */

ComputePipeline::ComputePipeline(LLGL::ComputePipeline* native) :
    native_ { native }
{
}

LLGL::ComputePipeline* ComputePipeline::Native::get()
{
    return native_;
};


/*
 * ResourceHeap class
 */

ResourceHeap::ResourceHeap(LLGL::ResourceHeap* native) :
    native_ { native }
{
}

LLGL::ResourceHeap* ResourceHeap::Native::get()
{
    return native_;
};


/*
 * BufferArray class
 */

BufferArray::BufferArray(LLGL::BufferArray* native) :
    native_ { native }
{
}

LLGL::BufferArray* BufferArray::Native::get()
{
    return native_;
};


} // /namespace SharpLLGL



// ================================================================================
