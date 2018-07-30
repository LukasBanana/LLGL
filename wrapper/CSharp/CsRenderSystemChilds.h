/*
 * CsRenderSystemChilds.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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
#include <LLGL/GraphicsPipeline.h>
#include <LLGL/ComputePipeline.h>

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace LHermanns
{

namespace LLGL
{


public enum class ResourceType
{
    Undefined,
    VertexBuffer,
    IndexBuffer,
    ConstantBuffer,
    StorageBuffer,
    StreamOutputBuffer,
    Texture,
    Sampler,
};

public enum class TextureType
{
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCube,
    Texture1DArray,
    Texture2DArray,
    TextureCubeArray,
    Texture2DMS,
    Texture2DMSArray,
};

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

        Resource(::LLGL::Resource* native);

        property ::LLGL::Resource* Native
        {
            ::LLGL::Resource* get();
        }

        /// <summary>Returns the resource type enumeration entry for this resource object.</summary>
        property ResourceType ResourceType
        {
            LHermanns::LLGL::ResourceType get();
        }

    private:

        ::LLGL::Resource* native_ = nullptr;

};

public ref class Buffer : public Resource
{

    public:

        Buffer(::LLGL::Buffer* native);

        property ::LLGL::Buffer* NativeSub
        {
            ::LLGL::Buffer* get();
        };

};

public ref class Texture : public Resource
{

    public:

        Texture(::LLGL::Texture* native);

        property ::LLGL::Texture* NativeSub
        {
            ::LLGL::Texture* get();
        };

        property TextureType Type
        {
            TextureType get();
        };

};

public ref class Sampler : public Resource
{

    public:

        Sampler(::LLGL::Sampler* native);

        property ::LLGL::Sampler* NativeSub
        {
            ::LLGL::Sampler* get();
        };

};

public ref class QueryHeap
{

    public:

        QueryHeap(::LLGL::QueryHeap* native);

        property ::LLGL::QueryHeap* Native
        {
            ::LLGL::QueryHeap* get();
        };

        property QueryType Type
        {
            QueryType get();
        };

    private:

        ::LLGL::QueryHeap* native_ = nullptr;

};

public ref class Fence
{

    public:

        Fence(::LLGL::Fence* native);

        property ::LLGL::Fence* Native
        {
            ::LLGL::Fence* get();
        };

    private:

        ::LLGL::Fence* native_ = nullptr;

};

public ref class RenderPass
{

    public:

        RenderPass(::LLGL::RenderPass* native);

        property ::LLGL::RenderPass* Native
        {
            ::LLGL::RenderPass* get();
        };

    private:

        ::LLGL::RenderPass* native_ = nullptr;

};

public ref class PipelineLayout
{

    public:

        PipelineLayout(::LLGL::PipelineLayout* native);

        property ::LLGL::PipelineLayout* Native
        {
            ::LLGL::PipelineLayout* get();
        };

    private:

        ::LLGL::PipelineLayout* native_ = nullptr;

};

public ref class GraphicsPipeline
{

    public:

        GraphicsPipeline(::LLGL::GraphicsPipeline* native);

        property ::LLGL::GraphicsPipeline* Native
        {
            ::LLGL::GraphicsPipeline* get();
        };

    private:

        ::LLGL::GraphicsPipeline* native_ = nullptr;

};

public ref class ComputePipeline
{

    public:

        ComputePipeline(::LLGL::ComputePipeline* native);

        property ::LLGL::ComputePipeline* Native
        {
            ::LLGL::ComputePipeline* get();
        };

    private:

        ::LLGL::ComputePipeline* native_ = nullptr;

};

public ref class ResourceHeap
{

    public:

        ResourceHeap(::LLGL::ResourceHeap* native);

        property ::LLGL::ResourceHeap* Native
        {
            ::LLGL::ResourceHeap* get();
        };

    private:

        ::LLGL::ResourceHeap* native_ = nullptr;

};

public ref class BufferArray
{

    public:

        BufferArray(::LLGL::BufferArray* native);

        property ::LLGL::BufferArray* Native
        {
            ::LLGL::BufferArray* get();
        };

    private:

        ::LLGL::BufferArray* native_ = nullptr;

};


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
