/*
 * VKCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_CORE_H
#define LLGL_VK_CORE_H


#include "Vulkan.h"
#include <string>


namespace LLGL
{


/* ----- Functions ----- */

// Converts the DX error code into a string.
std::string VKErrorToStr(const VkResult errorCode);

// Throws an std::runtime_error exception if 'errorCode' is not VK_SUCCESS.
void VKThrowIfFailed(const VkResult errorCode, const std::string& info);

// Converts the specified Vulkan API version into a string (e.g. "1.0.100").
std::string VKApiVersionToString(std::uint32_t version);


} // /namespace LLGL


#endif



// ================================================================================
