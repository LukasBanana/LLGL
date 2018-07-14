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

public ref class Resource
{

    public:

        Resource(::LLGL::Resource* instance);

        property ResourceType ResourceType
        {
            LHermanns::LLGL::ResourceType get();
        }

        property void* Native
        {
            void* get();
        }

    private:

        ::LLGL::Resource* instance_ = nullptr;

};

public ref class Buffer : public Resource
{

    public:

        Buffer(::LLGL::Buffer* instance) :
            Resource { instance }
        {
        }

};

public ref class Texture : public Resource
{

    public:

        Texture(::LLGL::Texture* instance) :
            Resource { instance }
        {
        }

};

public ref class Sampler : public Resource
{

    public:

        Sampler(::LLGL::Sampler* instance) :
            Resource { instance }
        {
        }

};



} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
