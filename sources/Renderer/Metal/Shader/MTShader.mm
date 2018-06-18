/*
 * MTShader.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTShader.h"


namespace LLGL
{


MTShader::MTShader(id<MTLDevice> device, const ShaderType type) :
    Shader  { type   },
    device_ { device }
{
}

MTShader::~MTShader()
{
    ReleaseError();
    if (native_)
        [native_ release];
    if (library_)
        [library_ release];
}

static MTLLanguageVersion GetMTLLanguageVersion(const ShaderDescriptor& desc)
{
    if (desc.target == "2.0")
        return MTLLanguageVersion2_0;
    if (desc.target == "1.2")
        return MTLLanguageVersion1_2;
    return MTLLanguageVersion1_1;
}

bool MTShader::Compile(const std::string& sourceCode, const ShaderDescriptor& shaderDesc)
{
    /* Convert strings to NSString */
    NSString* sourceString = [[NSString alloc] initWithUTF8String:sourceCode.c_str()];
    NSString* entryPoint = [[NSString alloc] initWithUTF8String:shaderDesc.entryPoint.c_str()];

    /* Initialize shader compiler options */
    MTLCompileOptions* opt = [MTLCompileOptions alloc];
    [opt setLanguageVersion:GetMTLLanguageVersion(shaderDesc)];

    /* Load shader library */
    ReleaseError();
    error_ = [NSError alloc];
    
    library_ = [device_
        newLibraryWithSource:   sourceString
        options:                opt
        error:                  &error_
    ];
    
    bool result = false;
    
    if (library_)
    {
        /* Create shader function */
        native_ = [library_ newFunctionWithName:entryPoint];
        if (native_)
        {
            result = true;
            ReleaseError();
        }
    }
    
    /* Release NSString objects */
    [sourceString release];
    [entryPoint release];
    
    return result;
}

bool MTShader::LoadBinary(std::vector<char>&& binaryCode, const ShaderDescriptor& shaderDesc)
{
    return false; // dummy
}

std::string MTShader::Disassemble(int flags)
{
    return ""; // dummy
}

std::string MTShader::QueryInfoLog()
{
    std::string s;
    
    if (error_ != nullptr)
    {
        NSString* errorMsg = [error_ localizedDescription];
        s = [errorMsg cStringUsingEncoding:NSUTF8StringEncoding];
        [errorMsg release];
    }
    
    return s;
}


/*
 * ======= Private: =======
 */

void MTShader::ReleaseError()
{
    if (error_ != nullptr)
    {
        [error_ release];
        error_ = nullptr;
    }
}


} // /namespace LLGL



// ================================================================================
