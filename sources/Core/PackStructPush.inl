/*
 * PackStructPush.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#if defined _MSC_VER
#   pragma pack(push, packing)
#   pragma pack(1)
#   define LLGL_PACK_STRUCT
#elif defined __GNUC__ || defined __clang__
#   define LLGL_PACK_STRUCT __attribute__((packed))
#else
#   define LLGL_PACK_STRUCT
#endif



// ================================================================================
