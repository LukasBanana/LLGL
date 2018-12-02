/*
 * PackPop.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

/* <--- THIS FILE MUST NOT HAVE A HEADER GUARD! ---> */

#ifdef LLGL_PACK_STRUCT
#   ifdef _MSC_VER
#       pragma pack(pop, packing)
#   endif
#   undef LLGL_PACK_STRUCT
#else
#   error LLGL_PACK_STRUCT undefined: "PackPop.h" included but no preceding "PackPush.h"!
#endif



// ================================================================================
