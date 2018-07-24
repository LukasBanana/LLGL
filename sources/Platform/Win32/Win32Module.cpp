/*
 * Win32Module.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Win32Module.h"
#include "../../Core/Helper.h"


namespace LLGL
{


std::string Module::GetModuleFilename(std::string moduleName)
{
    /* Extend module name to Win32 dynamic link library name (DLL) */
    #ifdef LLGL_DEBUG
    moduleName += "D";
    #endif
    return ("LLGL_" + moduleName + ".dll");
}

// Call Win32 function 'LoadLibrary' but with dialog error messages disabled
static HMODULE LoadLibrarySafe(LPCSTR filename)
{
    /* Disable dialog error messages */
    UINT prevMode = SetErrorMode(0);
    SetErrorMode(prevMode | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    /* Load library */
    HMODULE module = LoadLibraryA(filename);

    /* Reset previous error mode */
    SetErrorMode(prevMode);

    return module;
}

bool Module::IsAvailable(const std::string& moduleFilename)
{
    /* Check if Win32 dynamic link library can be loaded properly */
    if (auto handle = LoadLibrarySafe(moduleFilename.c_str()))
    {
        FreeLibrary(handle);
        return true;
    }
    return false;
}

std::unique_ptr<Module> Module::Load(const std::string& moduleFilename)
{
    return MakeUnique<Win32Module>(moduleFilename);
}

Win32Module::Win32Module(const std::string& moduleFilename)
{
    /* Open Win32 dynamic link library (DLL) */
    handle_ = LoadLibrarySafe(moduleFilename.c_str());

    /* Check if loading has failed */
    if (!handle_)
        throw std::runtime_error("failed to load dynamic link library (DLL): \"" + moduleFilename + "\"");
}

Win32Module::~Win32Module()
{
    FreeLibrary(handle_);
}

void* Win32Module::LoadProcedure(const std::string& procedureName)
{
    /* Get procedure address from library module and return it as raw-pointer */
    auto procAddr = GetProcAddress(handle_, procedureName.c_str());
    return reinterpret_cast<void*>(procAddr);
}


} // /namespace LLGL



// ================================================================================
