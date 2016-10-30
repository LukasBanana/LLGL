/*
 * Platform.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_PLATFORM_H
#define LLGL_PLATFORM_H


// Macro for OS

#if defined(_WIN32)
#   define LLGL_OS_WIN32
#elif defined(__APPLE__)
#   include "TargetConditionals.h"
#   if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#       define LLGL_OS_IOS
#   else
#       define LLGL_OS_MACOS
#   endif
#elif defined(__linux__)
#   define LLGL_OS_LINUX
#endif


// Macro for CPU architecture
// see https://sourceforge.net/p/predef/wiki/Architectures/

#if defined(_M_ARM) || defined(__arm__)
#   define LLGL_ARCH_ARM
#elif defined(_M_X64) || defined(__amd64__)
#   define LLGL_ARCH_X64
#elif defined(_M_IX86) || defined(_X86_) || defined(__X86__) || defined(__i386__)
#   define LLGL_ARCH_X86
#endif


#endif



// ================================================================================
