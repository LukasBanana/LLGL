/*
 * FragmentAttribute.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_FRAGMENT_ATTRIBUTE_H
#define LLGL_C99_FRAGMENT_ATTRIBUTE_H


#include <LLGL-C/Format.h>
#include <LLGL-C/SystemValue.h>
#include <stdint.h>


/* ----- Structures ----- */

typedef struct LLGLFragmentAttribute
{
    const char*     name;
    LLGLFormat      format;
    uint32_t        location;
    LLGLSystemValue systemValue;
}
LLGLFragmentAttribute;


#endif



// ================================================================================
