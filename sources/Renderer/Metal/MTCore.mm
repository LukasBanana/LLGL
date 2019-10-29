/*
 * MTCore.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

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
        
        throw std::runtime_error(s);
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
