/*
 * Module.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MODULE_H
#define LLGL_MODULE_H


#include <LLGL/Export.h>
#include <LLGL/NonCopyable.h>
#include <LLGL/Report.h>
#include <memory>
#include <string>


namespace LLGL
{


#ifdef _WIN32
#   define LLGL_PROC_INTERFACE(RET, NAME, ARG_LIST) typedef RET (__cdecl *NAME) ARG_LIST
#else
#   define LLGL_PROC_INTERFACE(RET, NAME, ARG_LIST) typedef RET (*NAME) ARG_LIST
#endif


// Module class (to load procedures for shared libraries)
class LLGL_EXPORT Module : public NonCopyable
{

    public:

        // Converts the module name into a specific filename (e.g. "OpenGL" to "LLGL_OpenGL.dll" on Windows).
        static std::string GetModuleFilename(const char* moduleName);

        // Returns true if the specified module is available.
        static bool IsAvailable(const char* moduleFilename);

        // Returns the specified module or null if it is not available.
        static std::unique_ptr<Module> Load(const char* moduleFilename, Report* report = nullptr);

    public:

        // Returns a raw pointer to the specified procedure loaded from this module.
        virtual void* LoadProcedure(const char* procedureName) = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
