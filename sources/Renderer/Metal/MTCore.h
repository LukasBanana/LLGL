/*
 * MTCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
