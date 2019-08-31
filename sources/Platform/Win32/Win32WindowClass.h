/*
 * Win32WindowClass.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        static Win32WindowClass* Instance();

        const wchar_t* GetName() const;

    private:

        Win32WindowClass();

};


} // /namespace LLGL


#endif



// ================================================================================
