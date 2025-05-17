/*
 * LinuxModule.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "LinuxModule.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Exception.h"
#include <dlfcn.h>
#include <unistd.h>
#include <stdexcept>
#include <errno.h>
#include <string.h>


namespace LLGL
{


std::string Module::GetModuleFilename(const char* moduleName)
{
    /* Extend module name to Linux shared library name (SO) */
    std::string s = "libLLGL_";
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
    std::unique_ptr<LinuxModule> module = MakeUnique<LinuxModule>(moduleFilename, report);
    return (module->IsValid() ? std::move(module) : nullptr);
}

LinuxModule::LinuxModule(const char* moduleFilename, Report* report)
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

LinuxModule::~LinuxModule()
{
    if (handle_ != nullptr)
        dlclose(handle_);
}

void* LinuxModule::LoadProcedure(const char* procedureName)
{
    /* Get procedure address from library module and return it as raw-pointer */
    return dlsym(handle_, procedureName);
}


} // /namespace LLGL



// ================================================================================
