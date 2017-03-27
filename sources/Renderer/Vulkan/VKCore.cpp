/*
 * VKCore.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKCore.h"
#include "../../Core/Helper.h"
#include "../../Core/HelperMacros.h"


namespace LLGL
{


std::string VKErrorToStr(const VkResult errorCode)
{
    // see https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkResult.html
    switch (errorCode)
    {
        LLGL_CASE_TO_STR( VK_SUCCESS );
        LLGL_CASE_TO_STR( VK_NOT_READY );
        LLGL_CASE_TO_STR( VK_TIMEOUT );
        LLGL_CASE_TO_STR( VK_EVENT_SET );
        LLGL_CASE_TO_STR( VK_EVENT_RESET );
        LLGL_CASE_TO_STR( VK_INCOMPLETE );
        LLGL_CASE_TO_STR( VK_ERROR_OUT_OF_HOST_MEMORY );
        LLGL_CASE_TO_STR( VK_ERROR_OUT_OF_DEVICE_MEMORY );
        LLGL_CASE_TO_STR( VK_ERROR_INITIALIZATION_FAILED );
        LLGL_CASE_TO_STR( VK_ERROR_DEVICE_LOST );
        LLGL_CASE_TO_STR( VK_ERROR_MEMORY_MAP_FAILED );
        LLGL_CASE_TO_STR( VK_ERROR_LAYER_NOT_PRESENT );
        LLGL_CASE_TO_STR( VK_ERROR_EXTENSION_NOT_PRESENT );
        LLGL_CASE_TO_STR( VK_ERROR_FEATURE_NOT_PRESENT );
        LLGL_CASE_TO_STR( VK_ERROR_INCOMPATIBLE_DRIVER );
        LLGL_CASE_TO_STR( VK_ERROR_TOO_MANY_OBJECTS );
        LLGL_CASE_TO_STR( VK_ERROR_FORMAT_NOT_SUPPORTED );
        LLGL_CASE_TO_STR( VK_ERROR_SURFACE_LOST_KHR );
        LLGL_CASE_TO_STR( VK_ERROR_NATIVE_WINDOW_IN_USE_KHR );
        LLGL_CASE_TO_STR( VK_SUBOPTIMAL_KHR );
        LLGL_CASE_TO_STR( VK_ERROR_OUT_OF_DATE_KHR );
        LLGL_CASE_TO_STR( VK_ERROR_INCOMPATIBLE_DISPLAY_KHR );
        LLGL_CASE_TO_STR( VK_ERROR_VALIDATION_FAILED_EXT );
        LLGL_CASE_TO_STR( VK_ERROR_INVALID_SHADER_NV );
        LLGL_CASE_TO_STR( VK_RESULT_RANGE_SIZE );
    }
    return ToHex(errorCode);
}

void VKThrowIfFailed(const VkResult errorCode, const std::string& info)
{
    if (errorCode != VK_SUCCESS)
        throw std::runtime_error(info + " (error code = " + VKErrorToStr(errorCode) + ")");
}


} // /namespace LLGL



// ================================================================================
