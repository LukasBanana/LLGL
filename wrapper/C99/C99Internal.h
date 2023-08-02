/*
 * C99Internal.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_INTERNAL_H
#define LLGL_C99_INTERNAL_H


#include "../../sources/Core/Assertion.h"


#define LLGL_PTR(TYPE, OBJ) \
    ((TYPE*)((OBJ).internal))

#define LLGL_REF(TYPE, OBJ) \
    (*((TYPE*)((OBJ).internal)))


#endif



// ================================================================================
