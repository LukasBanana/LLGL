/*
 * LinuxModule.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "LinuxModule.h"
#include "../../Core/CoreUtils.h"
#include <dlfcn.h>
#include <unistd.h>
#include <stdexcept>


namespace LLGL
{


// Returns absolute path of program instance
static std::string GetProgramPath()
{
    /* Get filename of running program */
    char buf[1024] = { 0 };
    (void)readlink("/proc/self/exe", buf, sizeof(buf));

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
