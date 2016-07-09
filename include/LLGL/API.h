/*
 * API.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_API_H__
#define __LLGL_API_H__


#ifdef _MSC_VER
#   define LLGL_EXPORT __declspec(dllexport)
#else
#   define LLGL_EXPORT
#endif

#ifdef LLGL_USE_STDCALL
#   define LLGL_API _stdcall
#else
#   define LLGL_API
#endif


#endif



// ================================================================================
