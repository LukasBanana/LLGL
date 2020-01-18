/*
 * LinuxModule.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "LinuxModule.h"
#include "../../Core/Helper.h"
#include <dlfcn.h>
#include <unistd.h>


namespace LLGL
{


// Returns absolute path of program instance
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

std::string Module::GetModuleFilename(const char* moduleName)
{
    /* Extend module name to Linux shared library name (SO) */
    std::string s = GetProgramPath();
    s += "libLLGL_";
    s += moduleName;
    #ifdef LLGL_DEBUG
    s += "D";
    #endif
    s += ".so";
    return s;
}

bool Module::IsAvailable(const char* moduleFilename)
{
    /* Check if Linux shared library can be loaded properly */
    if (auto handle = dlopen(moduleFilename, RTLD_LAZY))
    {
        dlclose(handle);
        return true;
    }
    return false;
}

std::unique_ptr<Module> Module::Load(const char* moduleFilename)
{
    return MakeUnique<LinuxModule>(moduleFilename);
}

LinuxModule::LinuxModule(const char* moduleFilename)
{
    /* Open Linux shared library */
    handle_ = dlopen(moduleFilename, RTLD_LAZY);

    /* Check if loading has failed */
    if (!handle_)
    {
        std::string msg = "failed to load shared library (SO): \"";
        msg += moduleFilename;
        msg += "\"";

        /* Append error message from most recent call to 'dlopen' */
        if (const char* err = dlerror())
        {
            msg += "; ";
            msg += err;
        }

        /* Throw error message */
        throw std::runtime_error(msg);
    }
}

LinuxModule::~LinuxModule()
{
    dlclose(handle_);
}

void* LinuxModule::LoadProcedure(const char* procedureName)
{
    /* Get procedure address from library module and return it as raw-pointer */
    auto procAddr = dlsym(handle_, procedureName);
    return reinterpret_cast<void*>(procAddr);
}


} // /namespace LLGL



// ================================================================================
