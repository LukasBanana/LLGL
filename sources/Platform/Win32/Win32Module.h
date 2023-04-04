/*
 * Win32Module.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WIN32_MODULE_H
#define LLGL_WIN32_MODULE_H


#include "../Module.h"
#include <Windows.h>


namespace LLGL
{


class Win32Module final : public Module
{

    public:

        Win32Module(const char* moduleFileanme);
        ~Win32Module();

        void* LoadProcedure(const char* procedureName) override;

    private:

        HMODULE handle_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
