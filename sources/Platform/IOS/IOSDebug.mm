/*
 * IOSDebug.mm
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
