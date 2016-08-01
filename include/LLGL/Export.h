/*
 * Export.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_EXPORT_H__
#define __LLGL_EXPORT_H__


#ifdef _MSC_VER
#   define LLGL_EXPORT __declspec(dllexport)
#else
#   define LLGL_EXPORT
#endif


#endif



// ================================================================================
