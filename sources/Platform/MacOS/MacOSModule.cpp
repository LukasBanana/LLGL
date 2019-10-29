/*
 * MacOSModule.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MacOSModule.h"
#include "../../Core/Helper.h"
#include <dlfcn.h>


namespace LLGL
{


std::string Module::GetModuleFilename(const char* moduleName)
{
    /* Extend module name to MacOS dynamic library name (DYLIB) */
    std::string s = "libLLGL_";
    s += moduleName;
    #ifdef LLGL_DEBUG
    s += "D";
    #endif
    s += ".dylib";
    return s;
}

bool Module::IsAvailable(const char* moduleFilename)
{
    /* Check if MacOS dynamic library can be loaded properly */
    auto handle = dlopen(moduleFilename, RTLD_LAZY);
    if (handle)
    {
        dlclose(handle);
        return true;
    }
    return false;
}

std::unique_ptr<Module> Module::Load(const char* moduleFilename)
{
    return MakeUnique<MacOSModule>(moduleFilename);
}

MacOSModule::MacOSModule(const char* moduleFilename)
{
    /* Open MacOS dynamic library */
    handle_ = dlopen(moduleFilename, RTLD_LAZY);

    /* Check if loading has failed */
    if (!handle_)
        throw std::runtime_error("failed to load dynamic library (DYLIB): \"" + std::string(moduleFilename) + "\"");
}

MacOSModule::~MacOSModule()
{
    dlclose(handle_);
}

void* MacOSModule::LoadProcedure(const char* procedureName)
{
    /* Get procedure address from library module and return it as raw-pointer */
    auto procAddr = dlsym(handle_, procedureName);
    return reinterpret_cast<void*>(procAddr);
}


} // /namespace LLGL



// ================================================================================
