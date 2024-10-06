/*
 * CommandBuffer.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Runtime.InteropServices;

namespace LLGL
{
    public sealed class CommandBuffer : RenderSystemChild
    {
        internal NativeLLGL.CommandBuffer Native { get; private set; }

        internal override NativeLLGL.RenderSystemChild NativeChild
        {
            get
            {
                unsafe
                {
                    return new NativeLLGL.RenderSystemChild() { ptr = Native.ptr };
                }
            }
        }

        internal CommandBuffer(NativeLLGL.CommandBuffer native, string debugName = null)
        {
            Native = native;
            InitializeDebugName(debugName);
        }

        ~CommandBuffer()
        {
            NativeLLGL.ReleaseCommandBuffer(Native);
        }

        public void Begin()
        {
            NativeLLGL.Begin(Native);
        }

        public void End()
        {
            NativeLLGL.End();
        }

        public void Execute(CommandBuffer secondaryCommandBuffer)
        {
            NativeLLGL.Execute(secondaryCommandBuffer.Native);
        }

        public void UpdateBuffer(Buffer dstBuffer, long dstOffset, byte[] data)
        {
            unsafe
            {
                fixed (void* dataPtr = data)
                {
                    NativeLLGL.UpdateBuffer(dstBuffer.Native, dstOffset, dataPtr, (short)data.Length);
                }
            }
        }

        public unsafe void UpdateBufferUnsafe(Buffer dstBuffer, long dstOffset, void* data, int dataSize)
        {
            NativeLLGL.UpdateBuffer(dstBuffer.Native, dstOffset, data, (short)dataSize);
        }

        public void CopyBuffer(Buffer dstBuffer, long dstOffset, Buffer srcBuffer, long srcOffset, long size)
        {
            NativeLLGL.CopyBuffer(dstBuffer.Native, dstOffset, srcBuffer.Native, srcOffset, size);
        }

        public void CopyBufferFromTexture(Buffer dstBuffer, long dstOffset, Texture srcTexture, TextureRegion srcRegion, int rowStride = 0, int layerStride = 0)
        {
            NativeLLGL.CopyBufferFromTexture(dstBuffer.Native, dstOffset, srcTexture.Native, ref srcRegion, rowStride, layerStride);
        }

        public void FillBuffer(Buffer dstBuffer, long dstOffset, int value, long fillSize = long.MaxValue)
        {
            NativeLLGL.FillBuffer(dstBuffer.Native, dstOffset, value, fillSize);
        }

        public void CopyTexture(Texture dstTexture, TextureLocation dstLocation, Texture srcTexture, TextureLocation srcLocation, Extent3D extent)
        {
            NativeLLGL.CopyTexture(dstTexture.Native, ref dstLocation, srcTexture.Native, ref srcLocation, ref extent);
        }

        public void CopyTextureFromBuffer(Texture dstTexture, TextureRegion dstRegion, Buffer srcBuffer, long srcOffset, int rowStride = 0, int layerStride = 0)
        {
            NativeLLGL.CopyTextureFromBuffer(dstTexture.Native, ref dstRegion, srcBuffer.Native, srcOffset, rowStride, layerStride);
        }

        public void CopyTextureFromFramebuffer(Texture dstTexture, TextureRegion dstRegion, Offset2D srcOffset)
        {
            NativeLLGL.CopyTextureFromFramebuffer(dstTexture.Native, ref dstRegion, ref srcOffset);
        }

        public void GenerateMips(Texture texture)
        {
            NativeLLGL.GenerateMips(texture.Native);
        }

        public void GenerateMips(Texture texture, TextureSubresource subresource)
        {
            NativeLLGL.GenerateMipsRange(texture.Native, ref subresource);
        }

        public void SetViewport(Viewport viewport)
        {
            NativeLLGL.SetViewport(ref viewport);
        }

        public void SetViewports(Viewport[] viewports)
        {
            unsafe
            {
                fixed (Viewport* viewportsPtr = viewports)
                {
                    NativeLLGL.SetViewports(viewports.Length, viewportsPtr);
                }
            }
        }

        public void SetScissor(Scissor scissor)
        {
            NativeLLGL.SetScissor(ref scissor);
        }

        public void SetScissors(Scissor[] scissors)
        {
            unsafe
            {
                fixed (Scissor* scissorsPtr = scissors)
                {
                    NativeLLGL.SetScissors(scissors.Length, scissorsPtr);
                }
            }
        }

        public void SetVertexBuffer(Buffer buffer)
        {
            NativeLLGL.SetVertexBuffer(buffer.Native);
        }

        public void SetVertexBufferArray(BufferArray bufferArray)
        {
            NativeLLGL.SetVertexBufferArray(bufferArray.Native);
        }

        public void SetIndexBuffer(Buffer buffer)
        {
            NativeLLGL.SetIndexBuffer(buffer.Native);
        }

        public void SetIndexBuffer(Buffer buffer, Format format, long offset)
        {
            NativeLLGL.SetIndexBufferExt(buffer.Native, format, offset);
        }

        public void SetResourceHeap(ResourceHeap resourceHeap, int descriptorSet)
        {
            NativeLLGL.SetResourceHeap(resourceHeap.Native, descriptorSet);
        }

        public void SetResource(int descriptor, Resource resource)
        {
            NativeLLGL.SetResource(descriptor, resource.NativeBase);
        }

        [Obsolete("LLGL.CommandBuffer.ResetResourceSlots is deprecated since 0.04b; No need to reset resource slots manually anymore!")]
        public void ResetResourceSlots(ResourceType resourceType, int firstSlot, int numSlots, BindFlags bindFlags, StageFlags stageFlags)
        {
            NativeLLGL.ResetResourceSlots(resourceType, firstSlot, numSlots, (int)bindFlags, (int)stageFlags);
        }

        public void BeginRenderPass(RenderTarget renderTarget)
        {
            NativeLLGL.BeginRenderPass(renderTarget.Native);
        }

        public void BeginRenderPass(RenderTarget renderTarget, RenderPass renderPass, ClearValue[] clearValues = null, int swapBufferIndex = -1)
        {
            if (renderPass != null && clearValues != null)
            {
                unsafe
                {
                    var nativeClearValues = stackalloc NativeLLGL.ClearValue[clearValues.Length];
                    for (int i = 0; i < clearValues.Length; ++i)
                    {
                        clearValues[i].CopyTo(ref nativeClearValues[i]);
                    }
                    NativeLLGL.BeginRenderPassWithClear(renderTarget.Native, renderPass.Native, clearValues.Length, nativeClearValues, swapBufferIndex);
                }
            }
            else
            {
                NativeLLGL.BeginRenderPass(renderTarget.Native);
            }
        }

        public void EndRenderPass()
        {
            NativeLLGL.EndRenderPass();
        }

        public void Clear(ClearFlags flags, ClearValue clearValue)
        {
            var nativeClearValue = clearValue.Native;
            NativeLLGL.Clear((int)flags, ref nativeClearValue);
        }

        public void ClearAttachments(AttachmentClear[] attachments)
        {
            unsafe
            {
                var nativeAttachments = stackalloc NativeLLGL.AttachmentClear[attachments.Length];
                for (int i = 0; i < attachments.Length; ++i)
                {
                    nativeAttachments[i] = attachments[i].Native;
                }
                NativeLLGL.ClearAttachments(attachments.Length, nativeAttachments);
            }
        }

        public void SetPipelineState(PipelineState pipelineState)
        {
            NativeLLGL.SetPipelineState(pipelineState.Native);
        }

        public void SetBlendFactor(Color color)
        {
            unsafe
            {
                float* colorPtr = stackalloc float[4];
                colorPtr[0] = color.R;
                colorPtr[1] = color.G;
                colorPtr[2] = color.B;
                colorPtr[3] = color.A;
                NativeLLGL.SetBlendFactor(colorPtr);
            }
        }

        public void SetStencilReference(int reference, StencilFace stencilFace)
        {
            NativeLLGL.SetStencilReference(reference, stencilFace);
        }

        public void SetUniforms(int first, byte[] data)
        {
            unsafe
            {
                fixed (void* dataPtr = data)
                {
                    NativeLLGL.SetUniforms(first, dataPtr, (short)data.Length);
                }
            }
        }

        public unsafe void SetUniformsUnsafe(int first, void* data, int dataSize)
        {
            NativeLLGL.SetUniforms(first, data, (short)dataSize);
        }

        public void BeginQuery(QueryHeap queryHeap, int query)
        {
            NativeLLGL.BeginQuery(queryHeap.Native, query);
        }

        public void EndQuery(QueryHeap queryHeap, int query)
        {
            NativeLLGL.EndQuery(queryHeap.Native, query);
        }

        public void BeginRenderCondition(QueryHeap queryHeap, int query, RenderConditionMode mode)
        {
            NativeLLGL.BeginRenderCondition(queryHeap.Native, query, mode);
        }

        public void EndRenderCondition()
        {
            NativeLLGL.EndRenderCondition();
        }

        public void BeginStreamOutput(Buffer[] buffers)
        {
            unsafe
            {
                var nativeBuffers = stackalloc NativeLLGL.Buffer[buffers.Length];
                for (int i = 0; i < buffers.Length; ++i)
                {
                    nativeBuffers[i] = buffers[i].Native;
                }
                NativeLLGL.BeginStreamOutput(buffers.Length, nativeBuffers);
            }
        }

        public void EndStreamOutput()
        {
            NativeLLGL.EndStreamOutput();
        }

        public void Draw(int numVertices, int firstVertex)
        {
            NativeLLGL.Draw(numVertices, firstVertex);
        }

        public void DrawIndexed(int numIndices, int firstIndex)
        {
            NativeLLGL.DrawIndexed(numIndices, firstIndex);
        }

        public void DrawIndexed(int numIndices, int firstIndex, int vertexOffset)
        {
            NativeLLGL.DrawIndexedExt(numIndices, firstIndex, vertexOffset);
        }

        public void DrawInstanced(int numVertices, int firstVertex, int numInstances)
        {
            NativeLLGL.DrawInstanced(numVertices, firstVertex, numInstances);
        }

        public void DrawInstanced(int numVertices, int firstVertex, int numInstances, int firstInstance)
        {
            NativeLLGL.DrawInstancedExt(numVertices, firstVertex, numInstances, firstInstance);
        }

        public void DrawIndexedInstanced(int numIndices, int numInstances, int firstIndex)
        {
            NativeLLGL.DrawIndexedInstanced(numIndices, numInstances, firstIndex);
        }

        public void DrawIndexedInstanced(int numIndices, int numInstances, int firstIndex, int vertexOffset, int firstInstance)
        {
            NativeLLGL.DrawIndexedInstancedExt(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
        }

        public void DrawIndirect(Buffer buffer, long offset)
        {
            NativeLLGL.DrawIndirect(buffer.Native, offset);
        }

        public void DrawIndirect(Buffer buffer, long offset, int numCommands, int stride)
        {
            NativeLLGL.DrawIndirectExt(buffer.Native, offset, numCommands, stride);
        }

        public void DrawIndexedIndirect(Buffer buffer, long offset)
        {
            NativeLLGL.DrawIndexedIndirect(buffer.Native, offset);
        }

        public void DrawIndexedIndirect(Buffer buffer, long offset, int numCommands, int stride)
        {
            NativeLLGL.DrawIndexedIndirectExt(buffer.Native, offset, numCommands, stride);
        }

        public void DrawStreamOutput()
        {
            NativeLLGL.DrawStreamOutput();
        }

        public void Dispatch(int numWorkGroupsX, int numWorkGroupsY, int numWorkGroupsZ)
        {
            NativeLLGL.Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
        }

        public void DispatchIndirect(Buffer buffer, long offset)
        {
            NativeLLGL.DispatchIndirect(buffer.Native, offset);
        }

        public void PushDebugGroup(string name)
        {
            NativeLLGL.PushDebugGroup(name);
        }

        public void PopDebugGroup()
        {
            NativeLLGL.PopDebugGroup();
        }
    }
}




// ================================================================================
