/*
 * VKExtensionRegistry.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKExtensionRegistry.h"
#include "../../../Core/Exception.h"
#include <array>


namespace LLGL
{


static std::array<bool, static_cast<std::size_t>(VKExt::Count)> g_registeredExtensions { { false } };

void RegisterExtension(VKExt extension)
{
    g_registeredExtensions[static_cast<std::size_t>(extension)] = true;
}

bool HasExtension(const VKExt extension)
{
    return g_registeredExtensions[static_cast<std::size_t>(extension)];
}

void AssertExtension(const VKExt extension, const char* extensionName, const char* funcName)
{
    if (!HasExtension(extension))
        ThrowVKExtensionNotSupportedExcept(funcName, extensionName);
}


} // /namespace LLGL



// ================================================================================
