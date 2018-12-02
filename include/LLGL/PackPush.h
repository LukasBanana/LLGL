/*
 * PackPush.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

/* <--- THIS FILE MUST NOT HAVE A HEADER GUARD! ---> */

#if defined LLGL_PACK_STRUCT
#   error LLGL_PACK_STRUCT already defined: "PackPush.h" included but no succeeding "PackPop.h"!
#else
#   if defined _MSC_VER
#       pragma pack(push, packing)
#       pragma pack(1)
#       define LLGL_PACK_STRUCT
#   elif defined __GNUC__ || defined __clang__
#       define LLGL_PACK_STRUCT __attribute__((packed))
#   else
#       define LLGL_PACK_STRUCT
#   endif
#endif



// ================================================================================
