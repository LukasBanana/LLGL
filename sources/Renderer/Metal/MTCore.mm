/*
 * MTCore.mm
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../../Core/Exception.h"
#include "MTCore.h"
#include <string>
#include <stdexcept>


namespace LLGL
{


void MTThrowIfFailed(NSError* error, const char* info)
{
    if (error != nullptr)
    {
        std::string s = info;
        s += ": ";
        
        NSString* errorMsg = [error localizedDescription];
        s += [errorMsg cStringUsingEncoding:NSUTF8StringEncoding];

        LLGL_TRAP("%s", s.c_str());
    }
}

void MTThrowIfCreateFailed(NSError* error, const char* interfaceName, const char* contextInfo)
{
    if (error != nullptr)
    {
        std::string s;
        {
            s = "failed to create instance of <";
            s += interfaceName;
            s += '>';
            if (contextInfo != nullptr)
            {
                s += ' ';
                s += contextInfo;
            }
        }
        MTThrowIfFailed(error, s.c_str());
    }
}

BOOL MTBoolean(bool value)
{
    return (value ? YES : NO);
}


} // /namespace LLGL



// ================================================================================
