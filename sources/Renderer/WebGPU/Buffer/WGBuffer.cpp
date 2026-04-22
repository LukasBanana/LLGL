/*
 * WGBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGBuffer.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Backend/WebGPU/NativeHandle.h>


namespace LLGL
{


static WGPUBufferUsage GetWebGpuBufferUsage(long bindFlags, long cpuAccessFlags)
{
    WGPUBufferUsage usage = WGPUBufferUsage_CopyDst;

    if ((bindFlags & BindFlags::VertexBuffer) != 0)
        usage |= WGPUBufferUsage_Vertex;
    if ((bindFlags & BindFlags::IndexBuffer) != 0)
        usage |= WGPUBufferUsage_Index;
    if ((bindFlags & BindFlags::ConstantBuffer) != 0)
        usage |= WGPUBufferUsage_Uniform;
    if ((bindFlags & BindFlags::Sampled) != 0)
        usage |= WGPUBufferUsage_Storage | WGPUBufferUsage_TexelBuffer; //???
    if ((bindFlags & BindFlags::Storage) != 0)
        usage |= WGPUBufferUsage_Storage;
    if ((bindFlags & BindFlags::IndirectBuffer) != 0)
        usage |= WGPUBufferUsage_Indirect;
    if ((bindFlags & BindFlags::CopySrc) != 0)
        usage |= WGPUBufferUsage_CopySrc;
    if ((bindFlags & BindFlags::CopyDst) != 0)
        usage |= WGPUBufferUsage_CopyDst;
    if ((cpuAccessFlags & CPUAccessFlags::Read) != 0)
        usage |= WGPUBufferUsage_MapRead;
    if ((cpuAccessFlags & CPUAccessFlags::Write) != 0)
        usage |= WGPUBufferUsage_MapWrite;

    return usage;
}

WGBuffer::WGBuffer(WGPUDevice device, const BufferDescriptor& bufferDesc) :
    Buffer { bufferDesc.bindFlags },
    size_  { bufferDesc.size      }
{
    WGPUBufferDescriptor wgpuBufferDesc;
    {
        wgpuBufferDesc.nextInChain      = nullptr;
        wgpuBufferDesc.label            = WGPU_STRING_VIEW_INIT;
        wgpuBufferDesc.usage            = GetWebGpuBufferUsage(bufferDesc.bindFlags, bufferDesc.cpuAccessFlags);
        wgpuBufferDesc.size             = bufferDesc.size;
        wgpuBufferDesc.mappedAtCreation = false;
    }
    buffer_ = wgpuDeviceCreateBuffer(device, &wgpuBufferDesc);
    LLGL_ASSERT_PTR(buffer_);
}

WGBuffer::~WGBuffer()
{
    wgpuBufferRelease(buffer_);
}

BufferDescriptor WGBuffer::GetDesc() const
{
    BufferDescriptor outDesc;
    {
        outDesc.size        = wgpuBufferGetSize(buffer_);
        outDesc.bindFlags   = GetBindFlags();
        //todo
    }
    return outDesc;
}

bool WGBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (auto* nativeHandleWG = GetTypedNativeHandle<WebGPU::ResourceNativeHandle>(nativeHandle, nativeHandleSize))
    {
        nativeHandleWG->type    = WebGPU::ResourceNativeType::Buffer;
        nativeHandleWG->buffer  = GetNative();
        return true;
    }
    return false;
}


} // /namespace LLGL



// ================================================================================
