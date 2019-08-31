/*
 * Win32Module.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
