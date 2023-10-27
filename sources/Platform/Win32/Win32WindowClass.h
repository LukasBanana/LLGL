/*
 * Win32WindowClass.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WIN32_WINDOW_CLASS_H
#define LLGL_WIN32_WINDOW_CLASS_H


#include <Windows.h>


namespace LLGL
{


class Win32WindowClass
{

    public:

        Win32WindowClass(const Win32WindowClass&) = delete;
        Win32WindowClass& operator = (const Win32WindowClass&) = delete;

        ~Win32WindowClass();

        static Win32WindowClass* Get();

        const TCHAR* GetName() const;

    private:

        Win32WindowClass();

};


} // /namespace LLGL


#endif



// ================================================================================
