/*
 * CsRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <vector>
#include <LLGL/RenderSystem.h>
#include "CsRenderSystemFlags.h"
#include "CsRenderContext.h"
#include "CsRenderContextFlags.h"
#include "CsBufferFlags.h"
#include "CsTextureFlags.h"
#include "CsImageFlags.h"
#include "CsCommandQueue.h"
#include "CsShader.h"
#include "CsShaderProgram.h"
#include "CsPipelineLayoutFlags.h"
#include "CsPipelineStateFlags.h"
#include "CsResourceHeapFlags.h"
#include "CsSamplerFlags.h"
#include "CsRenderTargetFlags.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace SharpLLGL
{


public ref class RenderingDebugger
{

    internal:

        property LLGL::RenderingDebugger* Native
        {
            LLGL::RenderingDebugger* get();
        };

    public:

        RenderingDebugger();
        ~RenderingDebugger();

#if 0
    protected:

        virtual void OnError();
        virtual void OnWarning();
#endif

    private:

        LLGL::RenderingDebugger* native_ = nullptr;

};

public ref class RenderSystem
{

    public:

        /* ----- Common ----- */

        ~RenderSystem();

        static Collections::Generic::List<String^>^ FindModules();

        static RenderSystem^ Load(String^ moduleName);
        static RenderSystem^ Load(String^ moduleName, RenderingDebugger^ renderingDebugger);

        static void Unload(RenderSystem^ renderSystem);

        property int ID
        {
            int get();
        }

        property String^ Name
        {
            String^ get();
        }

        property RendererInfo^ Info
        {
            RendererInfo^ get();
        }

        #if 0

        property RenderingCapabilities^ RenderingCaps
        {
            RenderingCapabilities^ get();
        }

        property RenderSystemConfiguration^ Configuration
        {
            RenderSystemConfiguration^ get();
            void set(RenderSystemConfiguration^ config);
        }

        #endif

        /* ----- Render Context ----- */

        RenderContext^ CreateRenderContext(RenderContextDescriptor^ desc);

        //RenderContext^ CreateRenderContext(RenderContextDescriptor^ desc, Surface^ surface);

        void Release(RenderContext^ renderContext);

        /* ----- Command queues ----- */

        property CommandQueue^ CommandQueue
        {
            SharpLLGL::CommandQueue^ get();
        }

        /* ----- Command buffers ----- */

        CommandBuffer^ CreateCommandBuffer();

        void Release(CommandBuffer^ commandBuffer);

        /* ----- Buffers ------ */

        Buffer^ CreateBuffer(BufferDescriptor^ desc);

        generic <typename T>
        Buffer^ CreateBuffer(BufferDescriptor^ desc, array<T>^ initialData);

        BufferArray^ CreateBufferArray(array<Buffer^>^ bufferArray);

        void Release(Buffer^ buffer);

        void Release(BufferArray^ bufferArray);

        void WriteBuffer(Buffer^ dstBuffer, System::UInt64 dstOffset, System::IntPtr data, System::UInt64 dataSize);

        System::IntPtr MapBuffer(Buffer^ buffer, CPUAccess access);

        void UnmapBuffer(Buffer^ buffer);

        /* ----- Textures ----- */

        Texture^ CreateTexture(TextureDescriptor^ textureDesc);

        generic <typename T>
        Texture^ CreateTexture(TextureDescriptor^ textureDesc, SrcImageDescriptor<T>^ imageDesc);

        void Release(Texture^ texture);

        #if 0
        generic <typename T>
        void WriteTexture(Texture^ texture, TextureRegion^ textureRegion, SrcImageDescriptor<T>^ imageDesc);

        generic <typename T>
        void ReadTexture(Texture^ texture, unsigned int mipLevel, DstImageDescriptor<T>^ imageDesc);
        #endif

        /* ----- Samplers ---- */

        Sampler^ CreateSampler(SamplerDescriptor^ desc);

        void Release(Sampler^ sampler);

        /* ----- Resource Heaps ----- */

        ResourceHeap^ CreateResourceHeap(ResourceHeapDescriptor^ desc);

        void Release(ResourceHeap^ resourceHeap);

        /* ----- Render Targets ----- */

        RenderTarget^ CreateRenderTarget(RenderTargetDescriptor^ desc);

        void Release(RenderTarget^ renderTarget);

        /* ----- Shader ----- */

        Shader^ CreateShader(ShaderDescriptor^ desc);

        ShaderProgram^ CreateShaderProgram(ShaderProgramDescriptor^ desc);

        void Release(Shader^ shader);

        void Release(ShaderProgram^ shaderProgram);

        /* ----- Pipeline Layouts ----- */

        PipelineLayout^ CreatePipelineLayout(PipelineLayoutDescriptor^ desc);
        PipelineLayout^ CreatePipelineLayout(String^ layoutSignature);

        void Release(PipelineLayout^ pipelineLayout);

        /* ----- Pipeline States ----- */

        PipelineState^ CreatePipelineState(GraphicsPipelineDescriptor^ desc);
        #if 0
        PipelineState^ CreatePipelineState(ComputePipelineDescriptor^ desc);
        #endif

        void Release(PipelineState^ pipelineState);

        #if 0
        /* ----- Queries ----- */

        QueryHeap^ CreateQueryHeap(QueryHeapDescriptor^ desc);

        void Release(QueryHeap^ queryHeap);

        #endif

        /* ----- Fences ----- */

        Fence^ CreateFence();

        void Release(Fence^ fence);

    private:

        RenderSystem(std::unique_ptr<LLGL::RenderSystem>&& native);

    private:

        LLGL::RenderSystem*         native_         = nullptr;
        SharpLLGL::CommandQueue^    commandQueue_;

};


} // /namespace SharpLLGL



// ================================================================================
