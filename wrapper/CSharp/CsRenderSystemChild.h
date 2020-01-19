/*
 * CsRenderSystemChild.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <LLGL/Buffer.h>
#include <LLGL/Texture.h>
#include <LLGL/Sampler.h>
#include <LLGL/Fence.h>
#include <LLGL/QueryHeap.h>
#include <LLGL/RenderPass.h>
#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineState.h>
#include <LLGL/ResourceHeap.h>
#include "CsTextureFlags.h"
#include "CsBufferFlags.h"
#include "CsResourceFlags.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace SharpLLGL
{


public enum class QueryType
{
    SamplesPassed,
    AnySamplesPassed,
    AnySamplesPassedConservative,
    TimeElapsed,
    StreamOutPrimitivesWritten,
    StreamOutOverflow,
    PipelineStatistics,
};

public ref class Resource
{

    public:

        Resource(LLGL::Resource* native);

        /// <summary>Returns the resource type enumeration entry for this resource object.</summary>
        property ResourceType ResourceType
        {
            SharpLLGL::ResourceType get();
        }

    private:

        LLGL::Resource* native_ = nullptr;

    internal:

        property LLGL::Resource* Native
        {
            LLGL::Resource* get();
        }

};

public ref class Buffer : public Resource
{

    public:

        Buffer(LLGL::Buffer* native);

        property BindFlags BindFlags
        {
            SharpLLGL::BindFlags get();
        }

        #if 0//TODO
        property BufferDescriptor^ Desc
        {
            BufferDescriptor^ get();
        };
        #endif

    internal:

        property LLGL::Buffer* NativeSub
        {
            LLGL::Buffer* get();
        };

};

public ref class Texture : public Resource
{

    public:

        Texture(LLGL::Texture* native);

        property TextureType Type
        {
            TextureType get();
        };

        #if 0//TODO
        property TextureDescriptor^ Desc
        {
            TextureDescriptor^ get();
        };
        #endif

        property Format Format
        {
            SharpLLGL::Format get();
        }

        Extent3D^ GetMipExtent(unsigned int mipLevel);

    internal:

        property LLGL::Texture* NativeSub
        {
            LLGL::Texture* get();
        };

};

public ref class Sampler : public Resource
{

    public:

        Sampler(LLGL::Sampler* native);

    internal:

        property LLGL::Sampler* NativeSub
        {
            LLGL::Sampler* get();
        };

};

public ref class QueryHeap
{

    public:

        QueryHeap(LLGL::QueryHeap* native);

        property QueryType Type
        {
            QueryType get();
        };

    private:

        LLGL::QueryHeap* native_ = nullptr;

    internal:

        property LLGL::QueryHeap* Native
        {
            LLGL::QueryHeap* get();
        };

};

public ref class Fence
{

    public:

        Fence(LLGL::Fence* native);

    private:

        LLGL::Fence* native_ = nullptr;

    internal:

        property LLGL::Fence* Native
        {
            LLGL::Fence* get();
        };

};

public ref class RenderPass
{

    public:

        RenderPass(LLGL::RenderPass* native);

    private:

        LLGL::RenderPass* native_ = nullptr;

    internal:

        property LLGL::RenderPass* Native
        {
            LLGL::RenderPass* get();
        };

};

public ref class PipelineLayout
{

    public:

        PipelineLayout(LLGL::PipelineLayout* native);

    private:

        LLGL::PipelineLayout* native_ = nullptr;

    internal:

        property LLGL::PipelineLayout* Native
        {
            LLGL::PipelineLayout* get();
        };

};

public ref class PipelineState
{

    public:

        PipelineState(LLGL::PipelineState* native);

    private:

        LLGL::PipelineState* native_ = nullptr;

    internal:

        property LLGL::PipelineState* Native
        {
            LLGL::PipelineState* get();
        };

};

public ref class ResourceHeap
{

    public:

        ResourceHeap(LLGL::ResourceHeap* native);

        property unsigned int NumDescriptorSets
        {
            unsigned int get();
        };

    private:

        LLGL::ResourceHeap* native_ = nullptr;

    internal:

        property LLGL::ResourceHeap* Native
        {
            LLGL::ResourceHeap* get();
        };

};

public ref class BufferArray
{

    public:

        BufferArray(LLGL::BufferArray* native);

    private:

        LLGL::BufferArray* native_ = nullptr;

    internal:

        property LLGL::BufferArray* Native
        {
            LLGL::BufferArray* get();
        };

};


} // /namespace SharpLLGL



// ================================================================================
