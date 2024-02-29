/*
 * AndroidModule.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidModule.h"
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

    std::size_t pathEnd = path.find_last_of('/');
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
    if (void* handle = dlopen(moduleFilename, RTLD_LAZY))
    {
        dlclose(handle);
        return true;
    }
    return false;
}

std::unique_ptr<Module> Module::Load(const char* moduleFilename, Report* report)
{
    std::unique_ptr<AndroidModule> module = MakeUnique<AndroidModule>(moduleFilename, report);
    return (module->IsValid() ? std::move(module) : nullptr);
}

AndroidModule::AndroidModule(const char* moduleFilename, Report* report)
{
    /* Open Linux shared library */
    handle_ = dlopen(moduleFilename, RTLD_LAZY);

    /* Check if loading has failed */
    if (!handle_ && report != nullptr)
    {
        /* Append error message from most recent call to 'dlopen' */
        std::string appendix;
        if (const char* err = dlerror())
        {
            appendix += "; ";
            appendix += err;
        }

        /* Throw error message */
        report->Errorf("failed to load shared library (SO): \"%s\"%s\n", moduleFilename, appendix.c_str());
    }
}

AndroidModule::~AndroidModule()
{
    if (handle_ != nullptr)
        dlclose(handle_);
}

void* AndroidModule::LoadProcedure(const char* procedureName)
{
    /* Get procedure address from library module and return it as raw-pointer */
    return dlsym(handle_, procedureName);
}


} // /namespace LLGL



// ================================================================================