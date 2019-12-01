/*
 * Platform.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_PLATFORM_H
#define LLGL_PLATFORM_H


/*
Macros for OS
see https://sourceforge.net/p/predef/wiki/OperatingSystems/
*/

#if defined _WIN32
#   define LLGL_OS_WIN32
#elif defined __APPLE__
#   include <TargetConditionals.h>
#   if TARGET_OS_OSX != 0
#       define LLGL_OS_MACOS
#   elif TARGET_OS_IOS != 0
#       define LLGL_OS_IOS
#   endif
#elif defined __ANDROID__
#   define LLGL_OS_ANDROID
#elif defined __linux__
#   define LLGL_OS_LINUX
#endif


/*
Macros for CPU architecture
see https://sourceforge.net/p/predef/wiki/Architectures/
*/

#if defined _M_ARM || defined __arm__
#   define LLGL_ARCH_ARM
#elif defined _M_X64 || defined __amd64__
#   define LLGL_ARCH_AMD64
#elif defined _M_IX86 || defined _X86_ || defined __X86__ || defined __i386__
#   define LLGL_ARCH_IA32
#endif


/*
Macros for calling conventions
see https://clang.llvm.org/docs/AttributeReference.html#calling-conventions
( Note that AMD64 (x86_64) and ARM have a single unified calling convention
  see https://eli.thegreenplace.net/2011/09/06/stack-frame-layout-on-x86-64/#id8 )
*/

#ifdef LLGL_ARCH_IA32
#   if defined _MSC_VER
#       define LLGL_API
#       define LLGL_API_CDECL       __cdecl
#       define LLGL_API_STDCALL     __stdcall
#       define LLGL_API_THISCALL    __thiscall
#   elif defined __clang__ or defined __GNUC__
#       define LLGL_API
#       define LLGL_API_CDECL       __attribute__((cdecl))
#       define LLGL_API_STDCALL     __attribute__((stdcall))
#       define LLGL_API_THISCALL    __attribute__((thiscall)
#   endif
#else
#   define LLGL_API
#   define LLGL_API_CDECL
#   define LLGL_API_STDCALL
#   define LLGL_API_THISCALL
#endif


#endif



// ================================================================================
