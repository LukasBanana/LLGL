/*
 * AppUtils.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#import "AppUtils.h"
#import <UIKit/UIKit.h>


std::string FindNSResourcePath(const std::string& filename)
{
    NSString* filenameNS = [[NSString alloc] initWithCString:filename.c_str() encoding:NSASCIIStringEncoding];
    NSString* path = [[NSBundle mainBundle] pathForResource:filenameNS ofType:nil];
    [filenameNS release];
    return (path != nil ? std::string([path UTF8String]) : "");
}

