/*
 * LinuxModule.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "LinuxModule.h"
#include <dlfcn.h>
#include <unistd.h>


namespace LLGL
{


static std::string GetProgramPath()
{
    /* Get filename of running program */
    static const std::size_t bufLen = 1024;
    char buf[bufLen] = { 0 };
    readlink("/proc/self/exe", buf, bufLen);
    
    /* Get path from program */
    std::string path = buf;
    
    auto pathEnd = path.find_last_of('/');
    if (pathEnd != std::string::npos)
        path.resize(pathEnd + 1);
    
    return path;
}

std::string Module::GetModuleFilename(std::string moduleName)
{
    /* Extend module name to Linux shared library name (SO) */
    #ifdef AC_DEBUG
    moduleName += "D";
    #endif
    
    /* Return module filename with absolute path */
    return GetProgramPath() + "libLLGL_" + moduleName + ".so";
}

bool Module::IsAvailable(const std::string& moduleFilename)
{
    /* Check if Linux shared library can be loaded properly */
    if (auto handle = dlopen(moduleFilename.c_str(), RTLD_LAZY))
    {
        dlclose(handle);
        return true;
    }
    return false;
}

std::unique_ptr<Module> Module::Load(const std::string& moduleFilename)
{
    return std::unique_ptr<Module>(new LinuxModule(moduleFilename));
}

LinuxModule::LinuxModule(const std::string& moduleFilename)
{
    /* Open Linux shared library */
    handle_ = dlopen(moduleFilename.c_str(), RTLD_LAZY);

    /* Check if loading has failed */
    if (!handle_)
        throw std::runtime_error("failed to load shared library (SO) \"" + moduleFilename + "\"");
}

LinuxModule::~LinuxModule()
{
    dlclose(handle_);
}

void* LinuxModule::LoadProcedure(const std::string& procedureName)
{
    /* Get procedure address from library module and return it as raw-pointer */
    auto procAddr = dlsym(handle_, procedureName.c_str());
    return reinterpret_cast<void*>(procAddr);
}


} // /namespace LLGL



// ================================================================================
