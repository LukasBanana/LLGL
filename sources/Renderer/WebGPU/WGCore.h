/*
 * WGCore.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_CORE_H
#define LLGL_WG_CORE_H


#include <LLGL/Container/StringView.h>
#include <string.h>
#include <webgpu/webgpu.h>


namespace LLGL
{


struct WGCoreLimits
{
    std::uint32_t maxBindGroups;
    std::uint32_t maxBindingsPerBindGroup;
};

const char* ToString(WGPUAdapterType type);
const char* ToString(WGPUBackendType type);
const char* ToString(WGPUWaitStatus status);

void WGThrowIfCreateFailed(void* ptr, const char* interfaceName, const char* contextInfo = nullptr);

inline StringView ToStringView(WGPUStringView str)
{
    return StringView{ str.data, str.length };
}

inline WGPUStringView ToWGStringView(const char* text)
{
    if (text != nullptr)
        return WGPUStringView{ text, ::strlen(text) };
    else
        return WGPU_STRING_VIEW_INIT;
}


} // /namespace LLGL


#endif



// ================================================================================
