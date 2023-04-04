/*
 * IOSDebug.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Debug.h"
#include <stdio.h>

#import <Foundation/Foundation.h>


namespace LLGL
{


LLGL_EXPORT void DebugPuts(const char* text)
{
    #ifdef LLGL_DEBUG
    /* Print to Xcode debug console */
    NSLog(@"%@\n", @(text));
    #else
    /* Print to standard error stream */
    ::fprintf(stderr, "%s\n", text);
    #endif
}


} // /namespace LLGL



// ================================================================================
