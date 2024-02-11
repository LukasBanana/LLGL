/*
 * RenderSystem.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Runtime.InteropServices;

namespace LLGL
{
    public class RenderSystem
    {
        private int Id { get; set; }

        private static int CurrentId { get; set; }

        private NativeLLGL.ReportCallbackDelegate ReportCallbackDelegate { get; set; }

        private IntPtr LogHandle { get; set; }

        private RenderSystem(int id)
        {
            Id = id;

            // Initialize log callback
            unsafe
            {
                ReportCallbackDelegate = (NativeLLGL.ReportCallbackDelegate)(
                    (ReportType type, string text, void* userData) =>
                        (type == ReportType.Error ? Console.Error : Console.Out).Write(text));
                IntPtr logCallback = Marshal.GetFunctionPointerForDelegate(ReportCallbackDelegate);
                LogHandle = NativeLLGL.RegisterLogCallback(logCallback, null);
            }
        }

        ~RenderSystem()
        {
            NativeLLGL.UnloadRenderSystem();
            NativeLLGL.UnregisterLogCallback(LogHandle);
            RenderSystem.CurrentId = 0;
        }

        public static RenderSystem Load(RenderSystemDescriptor renderSystemDesc, Report report = null)
        {
            if (renderSystemDesc == null)
            {
                throw new ArgumentNullException("renderSystemDesc");
            }
            if (CurrentId != 0)
            {
                throw new InvalidOperationException("Cannot load multiple render systems in LLGL wrapper for C#");
            }

            try
            {
                var nativeDesc = renderSystemDesc.Native;
                int id = NativeLLGL.LoadRenderSystemExt(ref nativeDesc, report != null ? report.Native : new NativeLLGL.Report());
                if (id != 0)
                {
                    CurrentId = id;
                    return new RenderSystem(id);
                }
            }
            catch (System.DllNotFoundException e)
            {
                if (report != null)
                {
                    report.Errorf(e.ToString());
                }
            }

            return null;
        }

        public int RendererID
        {
            get
            {
                return NativeLLGL.GetRendererID();
            }
        }

        public string RendererName
        {
            get
            {
                return NativeLLGL.GetRendererName();
            }
        }

        public RendererInfo RendererInfo
        {
            get
            {
                var rendererInfoNative = new NativeLLGL.RendererInfo();
                NativeLLGL.GetRendererInfo(ref rendererInfoNative);
                return new RendererInfo(rendererInfoNative);
            }
        }

        public RenderingCapabilities RenderingCaps
        {
            get
            {
                var rendererCaps = new NativeLLGL.RenderingCapabilities();
                NativeLLGL.GetRenderingCaps(ref rendererCaps);
                return new RenderingCapabilities(rendererCaps);
            }
        }

        public Report Report
        {
            get
            {
                return new Report(NativeLLGL.GetRendererReport());
            }
        }

        public SwapChain CreateSwapChain(SwapChainDescriptor swapChainDesc, Surface surface = null)
        {
            var nativeSwapChainDesc = swapChainDesc.Native;
            if (surface != null)
            {
                return new SwapChain(NativeLLGL.CreateSwapChainExt(ref nativeSwapChainDesc, surface.NativeBase), swapChainDesc.DebugName);
            }
            else
            {
                return new SwapChain(NativeLLGL.CreateSwapChain(ref nativeSwapChainDesc), swapChainDesc.DebugName);
            }
        }

        public CommandQueue CommandQueue { get; private set; } = new CommandQueue();

        public CommandBuffer CreateCommandBuffer(CommandBufferDescriptor commandBufferDesc)
        {
            var nativeDesc = commandBufferDesc.Native;
            return new CommandBuffer(NativeLLGL.CreateCommandBuffer(ref nativeDesc), commandBufferDesc.DebugName);
        }

        #region GPU resources: buffers, textures, sampler states

        public Buffer CreateBuffer(BufferDescriptor bufferDesc, byte[] initialData = null)
        {
            var nativeDesc = bufferDesc.Native;
            unsafe
            {
                if (initialData != null)
                {
                    fixed (void* initialDataPtr = initialData)
                    {
                        return new Buffer(NativeLLGL.CreateBuffer(ref nativeDesc, initialDataPtr), bufferDesc.DebugName);
                    }
                }
                else
                {
                    return new Buffer(NativeLLGL.CreateBuffer(ref nativeDesc, null), bufferDesc.DebugName);
                }
            }
        }

        public unsafe Buffer CreateBufferUnsafe(BufferDescriptor bufferDesc, void* initialData)
        {
            var nativeDesc = bufferDesc.Native;
            return new Buffer(NativeLLGL.CreateBuffer(ref nativeDesc, initialData), bufferDesc.DebugName);
        }

        public void WriteBuffer(Buffer buffer, long offset, byte[] data)
        {
            unsafe
            {
                fixed (byte* dataPtr = data)
                {
                    NativeLLGL.WriteBuffer(buffer.Native, offset, dataPtr, data.Length);
                }
            }
        }

        public unsafe void WriteBufferUnsafe(Buffer buffer, long offset, void* data, long dataSize)
        {
            NativeLLGL.WriteBuffer(buffer.Native, offset, data, dataSize);
        }

        public void ReadBuffer(Buffer buffer, long offset, ref byte[] data)
        {
            unsafe
            {
                fixed (byte* dataPtr = data)
                {
                    NativeLLGL.ReadBuffer(buffer.Native, offset, dataPtr, data.Length);
                }
            }
        }

        public unsafe void ReadBufferUnsafe(Buffer buffer, long offset, void* data, long dataSize)
        {
            NativeLLGL.ReadBuffer(buffer.Native, offset, data, dataSize);
        }

        public unsafe void* MapBufferUnsafe(Buffer buffer, CPUAccess access)
        {
            return NativeLLGL.MapBuffer(buffer.Native, access);
        }

        public unsafe void* MapBufferRangeUnsafe(Buffer buffer, CPUAccess access, long offset, long length)
        {
            return NativeLLGL.MapBufferRange(buffer.Native, access, offset, length);
        }

        public void UnmapBuffer(Buffer buffer)
        {
            NativeLLGL.UnmapBuffer(buffer.Native);
        }

        public BufferArray CreateBufferArray(Buffer[] buffers)
        {
            unsafe
            {
                var nativeBuffers = stackalloc NativeLLGL.Buffer[buffers.Length];
                for (int i = 0; i < buffers.Length; ++i)
                {
                    nativeBuffers[i] = buffers[i].Native;
                }
                return new BufferArray(NativeLLGL.CreateBufferArray(buffers.Length, nativeBuffers));
            }
        }

        public Texture CreateTexture(TextureDescriptor textureDesc, ImageView imageView = null)
        {
            var nativeTextureDesc = textureDesc.Native;
            unsafe
            {
                if (imageView != null && imageView.IsValid)
                {
                    var nativeImageView = imageView.Native;
                    return new Texture(NativeLLGL.CreateTexture(ref nativeTextureDesc, &nativeImageView), textureDesc.DebugName);
                }
                else
                {
                    return new Texture(NativeLLGL.CreateTexture(ref nativeTextureDesc, null), textureDesc.DebugName);
                }
            }
        }

        public void WriteTexture(Texture texture, TextureRegion textureRegion, ImageView srcImageView)
        {
            unsafe
            {
                var nativeSrcImageView = srcImageView.Native;
                NativeLLGL.WriteTexture(texture.Native, ref textureRegion, ref nativeSrcImageView);
            }
        }

        public void ReadTexture(Texture texture, TextureRegion textureRegion, MutableImageView dstImageView)
        {
            unsafe
            {
                var nativeDstImageView = dstImageView.Native;
                NativeLLGL.ReadTexture(texture.Native, ref textureRegion, ref nativeDstImageView);
            }
        }

        public Sampler CreateSampler(SamplerDescriptor samplerDesc)
        {
            var nativeSamplerDesc = samplerDesc.Native;
            return new Sampler(NativeLLGL.CreateSampler(ref nativeSamplerDesc), samplerDesc.DebugName);
        }

        public ResourceHeap CreateResourceHeap(ResourceHeapDescriptor resourceHeapDesc, ResourceViewDescriptor[] resourceViews = null)
        {
            var nativeResourceHeapDesc = resourceHeapDesc.Native;
            if (resourceViews != null)
            {
                unsafe
                {
                    var nativeResourceViews = new NativeLLGL.ResourceViewDescriptor[resourceViews.Length];
                    for (int i = 0; i < resourceViews.Length; ++i)
                    {
                        nativeResourceViews[i] = resourceViews[i].Native;
                    }
                    fixed (NativeLLGL.ResourceViewDescriptor* nativeResourceViewsPtr = nativeResourceViews)
                    {
                        return new ResourceHeap(NativeLLGL.CreateResourceHeapExt(ref nativeResourceHeapDesc, (IntPtr)nativeResourceViews.Length, nativeResourceViewsPtr), resourceHeapDesc.DebugName);
                    }
                }
            }
            else
            {
                return new ResourceHeap(NativeLLGL.CreateResourceHeap(ref nativeResourceHeapDesc), resourceHeapDesc.DebugName);
            }
        }

        public void WriteResourceHeap(ResourceHeap resourceHeap, int firstDescriptor, ResourceViewDescriptor[] resourceViews)
        {
            unsafe
            {
                var nativeResourceViews = new NativeLLGL.ResourceViewDescriptor[resourceViews.Length];
                for (int i = 0; i < resourceViews.Length; ++i)
                {
                    nativeResourceViews[i] = resourceViews[i].Native;
                }
                fixed (NativeLLGL.ResourceViewDescriptor* nativeResourceViewsPtr = nativeResourceViews)
                {
                    NativeLLGL.WriteResourceHeap(resourceHeap.Native, firstDescriptor, (IntPtr)nativeResourceViews.Length, nativeResourceViewsPtr);
                }
            }
        }

        #endregion

        #region Pipeline states and shaders

        public PipelineLayout CreatePipelineLayout(PipelineLayoutDescriptor pipelineLayoutDesc)
        {
            var nativePipelineLayoutDesc = pipelineLayoutDesc.Native;
            return new PipelineLayout(NativeLLGL.CreatePipelineLayout(ref nativePipelineLayoutDesc), pipelineLayoutDesc.DebugName);
        }

        public PipelineCache CreatePipelineCache(byte[] initialBlob = null)
        {
            unsafe
            {
                if (initialBlob != null)
                {
                    fixed (void* initialBlobData = initialBlob)
                    {
                        return new PipelineCache(NativeLLGL.CreatePipelineCache(initialBlobData, (IntPtr)initialBlob.Length));
                    }
                }
                else
                {
                    return new PipelineCache(NativeLLGL.CreatePipelineCache(null, (IntPtr)0));
                }
            }
        }

        public PipelineState CreatePipelineState(GraphicsPipelineDescriptor pipelineStateDesc, PipelineCache pipelineCache = null)
        {
            var nativePipelineStateDesc = pipelineStateDesc.Native;
            if (pipelineCache != null)
            {
                return new PipelineState(NativeLLGL.CreateGraphicsPipelineStateExt(ref nativePipelineStateDesc, pipelineCache.Native));
            }
            else
            {
                return new PipelineState(NativeLLGL.CreateGraphicsPipelineState(ref nativePipelineStateDesc));
            }
        }

        public PipelineState CreatePipelineState(ComputePipelineDescriptor pipelineStateDesc, PipelineCache pipelineCache = null)
        {
            var nativePipelineStateDesc = pipelineStateDesc.Native;
            if (pipelineCache != null)
            {
                return new PipelineState(NativeLLGL.CreateComputePipelineStateExt(ref nativePipelineStateDesc, pipelineCache.Native));
            }
            else
            {
                return new PipelineState(NativeLLGL.CreateComputePipelineState(ref nativePipelineStateDesc));
            }
        }

        public Shader CreateShader(ShaderDescriptor shaderDesc)
        {
            var nativeShaderDesc = shaderDesc.Native;
            return new Shader(NativeLLGL.CreateShader(ref nativeShaderDesc), shaderDesc.DebugName);
        }

        #endregion

        public RenderPass CreateRenderPass(RenderPassDescriptor renderPassDesc)
        {
            var nativeRenderPassDesc = renderPassDesc.Native;
            return new RenderPass(NativeLLGL.CreateRenderPass(ref nativeRenderPassDesc), renderPassDesc.DebugName);
        }

        public RenderTarget CreateRenderTarget(RenderTargetDescriptor renderTargetDesc)
        {
            var nativeRenderTargetDesc = renderTargetDesc.Native;
            return new RenderTarget(NativeLLGL.CreateRenderTarget(ref nativeRenderTargetDesc), renderTargetDesc.DebugName);
        }

        public Fence CreateFence()
        {
            return new Fence(NativeLLGL.CreateFence());
        }

        public QueryHeap CreateQueryHeap(QueryHeapDescriptor queryHeapDesc)
        {
            var nativeQueryHeapDesc = queryHeapDesc.Native;
            return new QueryHeap(NativeLLGL.CreateQueryHeap(ref nativeQueryHeapDesc), queryHeapDesc.DebugName);
        }
    }
}




// ================================================================================
