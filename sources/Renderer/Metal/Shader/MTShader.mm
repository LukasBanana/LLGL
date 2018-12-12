/*
 * MTShader.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTShader.h"
#include "../../../Core/Exception.h"
#include <cstring>


namespace LLGL
{


MTShader::MTShader(id<MTLDevice> device, const ShaderDescriptor& desc) :
    Shader { desc.type }
{
    if (IsShaderSourceCode(desc.sourceType))
        hasErrors_ = CompileSource(device, desc);
    else
        ThrowNotSupportedExcept(__FUNCTION__, "binary shader source");
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
    if (desc.profile != nullptr)
    {
        if (std::strcmp(desc.profile, "2.0") == 0)
            return MTLLanguageVersion2_0;
        if (std::strcmp(desc.profile, "1.2") == 0)
            return MTLLanguageVersion1_2;
    }
    return MTLLanguageVersion1_1;
}

bool MTShader::HasErrors() const
{
    return hasErrors_;
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

bool MTShader::CompileSource(id<MTLDevice> device, const ShaderDescriptor& shaderDesc)
{
    /* Get source */
    NSString* sourceString = nil;
    
    if (shaderDesc.sourceType == ShaderSourceType::CodeFile)
    {
        NSString* filePath = [[NSString alloc] initWithUTF8String:shaderDesc.source];
        sourceString = [[NSString alloc] initWithContentsOfFile:filePath encoding:NSUTF8StringEncoding error:nil];
        [filePath release];
    }
    else
        sourceString = [[NSString alloc] initWithUTF8String:shaderDesc.source];
    
    if (sourceString == nil)
        throw std::runtime_error("cannot compile Metal shader without source");

    /* Convert entry point to string to NSString */
    NSString* entryPoint = [[NSString alloc] initWithUTF8String:(shaderDesc.entryPoint != nullptr ? shaderDesc.entryPoint : "")];

    /* Initialize shader compiler options */
    MTLCompileOptions* opt = [MTLCompileOptions alloc];
    [opt setLanguageVersion:GetMTLLanguageVersion(shaderDesc)];
    
    if ((shaderDesc.flags & (ShaderCompileFlags::O1 | ShaderCompileFlags::O2 | ShaderCompileFlags::O3)) != 0)
        [opt setFastMathEnabled:YES];

    /* Load shader library */
    ReleaseError();
    error_ = [NSError alloc];
    
    library_ = [device
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
        else
        {
            //TODO...
        }
    }
    
    /* Release NSString objects */
    [sourceString release];
    [entryPoint release];
    [opt release];
    
    return result;
}

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
