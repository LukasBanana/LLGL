/*
 * WGCore.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGCore.h"
#include "../../Core/Exception.h"
#include "../../Core/StringUtils.h"


namespace LLGL
{

    
const char* ToString(WGPUBackendType type)
{
    switch (type)
    {
        case WGPUBackendType_Undefined: return "<Undefined>";
        case WGPUBackendType_Null:      return "Null";
        case WGPUBackendType_WebGPU:    return "WebGPU";
        case WGPUBackendType_D3D11:     return "D3D11";
        case WGPUBackendType_D3D12:     return "D3D12";
        case WGPUBackendType_Metal:     return "Metal";
        case WGPUBackendType_Vulkan:    return "Vulkan";
        case WGPUBackendType_OpenGL:    return "OpenGL";
        case WGPUBackendType_OpenGLES:  return "OpenGLES";
        case WGPUBackendType_Force32:   return "<Force32>";
        default:                        return nullptr;
    }
}

const char* ToString(WGPUAdapterType type)
{
    switch (type)
    {
        case WGPUAdapterType_DiscreteGPU:   return "Discrete GPU";
        case WGPUAdapterType_IntegratedGPU: return "Integrated GPU";
        case WGPUAdapterType_CPU:           return "CPU";
        case WGPUAdapterType_Unknown:       return "Unknown";
        case WGPUAdapterType_Force32:       return "<Force32>";
        default:                            return nullptr;
    }
}

const char* ToString(WGPUWaitStatus status)
{
    switch (status)
    {
        case WGPUWaitStatus_Success:    return "Success";
        case WGPUWaitStatus_TimedOut:   return "TimedOut";
        case WGPUWaitStatus_Error:      return "Error";
        default:                        return IntToHex(static_cast<std::uint32_t>(status));
    }
}

void WGThrowIfCreateFailed(void* ptr, const char* interfaceName, const char* contextInfo)
{
    if (ptr == nullptr)
    {
        std::string s;
        {
            s = "failed to create instance of <";
            s += interfaceName;
            s += '>';
            if (contextInfo != nullptr)
            {
                s += ' ';
                s += contextInfo;
            }
        }
        LLGL_TRAP("%s", s.c_str());
    }
}


} // /namespace LLGL



// ================================================================================
