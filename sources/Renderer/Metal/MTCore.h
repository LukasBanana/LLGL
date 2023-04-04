/*
 * MTCore.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_CORE_H
#define LLGL_MT_CORE_H


#import <Metal/Metal.h>


namespace LLGL
{


// Throws an std::runtime_error exception if 'error' is not null.
void MTThrowIfFailed(NSError* error, const char* info);

// Throws an std::runtime_error exception if 'error' is not null.
void MTThrowIfCreateFailed(NSError* error, const char* interfaceName, const char* contextInfo = nullptr);

// Converts the specified C++ boolean (bool) to an Objective-C boolean (BOOL).
BOOL MTBoolean(bool value);


} // /namespace LLGL


#endif



// ================================================================================
