/*
 * IOSPath.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Path.h"
#include <LLGL/Utils/ForRange.h>

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>


namespace LLGL
{

namespace Path
{


LLGL_EXPORT char GetSeparator()
{
    return '/';
}

LLGL_EXPORT UTF8String GetWorkingDir()
{
    NSString* path = [[NSFileManager defaultManager] currentDirectoryPath];
    return (path != nullptr ? UTF8String{ [path UTF8String] } : "");
}

LLGL_EXPORT UTF8String GetAbsolutePath(const UTF8String& filename)
{
    NSString* filenameNS = [[NSString alloc] initWithBytes:filename.data() length:static_cast<NSUInteger>(filename.size()) encoding:NSASCIIStringEncoding];
    NSString* path = [[NSBundle mainBundle] pathForResource:filenameNS ofType:nil];
    [filenameNS release];
    return (path != nil ? UTF8String{ [path UTF8String] } : "");
}


} // /nameapace Path

} // /namespace LLGL



// ================================================================================
