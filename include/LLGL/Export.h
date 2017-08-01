/*
 * Export.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_EXPORT_H
#define LLGL_EXPORT_H


#if defined _MSC_VER && !defined LLGL_BUILD_STATIC_LIB
#   define LLGL_EXPORT __declspec(dllexport)
#else
#   define LLGL_EXPORT
#endif


#endif



// ================================================================================
