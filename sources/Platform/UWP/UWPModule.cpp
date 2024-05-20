/*
 * UWPModule.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "UWPModule.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/StringUtils.h"


namespace LLGL
{


std::string Module::GetModuleFilename(const char* moduleName)
{
    /* Extend module name to Win32 dynamic link library name (DLL) */
    std::string s = "LLGL_";
    s += moduleName;
    #ifdef LLGL_DEBUG
    s += "D";
    #endif
    s += ".dll";
    return s;
}

static HMODULE LoadLibrarySafe(LPCSTR filename)
{
    /* Load library */
    const std::wstring filenameUTF16 = ToWideString(filename);
    HMODULE module = LoadPackagedLibrary(filenameUTF16.c_str(), 0);
    return module;
}

bool Module::IsAvailable(const char* moduleFilename)
{
    /* Check if Win32 dynamic link library can be loaded properly */
    if (HMODULE handle = LoadLibrarySafe(moduleFilename))
    {
        FreeLibrary(handle);
        return true;
    }
    return false;
}

std::unique_ptr<Module> Module::Load(const char* moduleFilename, Report* report)
{
    std::unique_ptr<UWPModule> module = MakeUnique<UWPModule>(moduleFilename, report);
    return (module->IsValid() ? std::move(module) : nullptr);
}

UWPModule::UWPModule(const char* moduleFilename, Report* report)
{
    /* Open Win32 dynamic link library (DLL) */
    handle_ = LoadLibrarySafe(moduleFilename);

    /* Check if loading has failed */
    if (!handle_ && report != nullptr)
        report->Errorf("failed to load dynamic link library (DLL): \"%s\"\n", moduleFilename);
}

UWPModule::~UWPModule()
{
    if (handle_ != nullptr)
        FreeLibrary(handle_);
}

void* UWPModule::LoadProcedure(const char* procedureName)
{
    /* Get procedure address from library module and return it as raw-pointer */
    FARPROC procAddr = GetProcAddress(handle_, procedureName);
    return reinterpret_cast<void*>(procAddr);
}


} // /namespace LLGL



// ================================================================================
