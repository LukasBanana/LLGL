/*
 * VKDeviceMemoryManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKDeviceMemoryManager.h"
#include "../VKCore.h"


namespace LLGL
{


VKDeviceMemoryManager::VKDeviceMemoryManager(const VKPtr<VkDevice>& device) :
    device_ { device }
{
}


} // /namespace LLGL



// ================================================================================
