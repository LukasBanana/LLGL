/*
 * MTShader.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTShader.h"
#include <LLGL/Platform/Platform.h>
#include <cstring>


namespace LLGL
{


MTShader::MTShader(id<MTLDevice> device, const ShaderDescriptor& desc) :
    Shader { desc.type }
{
    hasErrors_ = !Compile(device, desc);
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
        if (std::strcmp(desc.profile, "2.1") == 0)
            return MTLLanguageVersion2_1;
        if (std::strcmp(desc.profile, "2.0") == 0)
            return MTLLanguageVersion2_0;
        if (std::strcmp(desc.profile, "1.2") == 0)
            return MTLLanguageVersion1_2;
        if (std::strcmp(desc.profile, "1.1") == 0)
            return MTLLanguageVersion1_1;
        #ifdef LLGL_OS_IOS
        if (std::strcmp(desc.profile, "1.0") == 0)
            return MTLLanguageVersion1_0;
        #endif
    }
    throw std::invalid_argument("invalid Metal shader version specified");
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
    }

    return s;
}


/*
 * ======= Private: =======
 */

bool MTShader::Compile(id<MTLDevice> device, const ShaderDescriptor& shaderDesc)
{
    if (IsShaderSourceCode(shaderDesc.sourceType))
        return CompileSource(device, shaderDesc);
    else
        return CompileBinary(device, shaderDesc);
}

static NSString* ToNSString(const char* s)
{
    return [[NSString alloc] initWithUTF8String:(s != nullptr ? s : "")];
}

static MTLCompileOptions* ToMTLCompileOptions(const ShaderDescriptor& shaderDesc)
{
    MTLCompileOptions* opt = [MTLCompileOptions alloc];

    [opt setLanguageVersion:GetMTLLanguageVersion(shaderDesc)];
    if ((shaderDesc.flags & (ShaderCompileFlags::O1 | ShaderCompileFlags::O2 | ShaderCompileFlags::O3)) != 0)
        [opt setFastMathEnabled:YES];

    return opt;
}

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

    /* Convert entry point to NSString, and initialize shader compile options */
    MTLCompileOptions* opt = ToMTLCompileOptions(shaderDesc);

    /* Load shader library */
    ReleaseError();
    error_ = [NSError alloc];

    library_ = [device
        newLibraryWithSource:   sourceString
        options:                opt
        error:                  &error_
    ];

    [sourceString release];
    [opt release];

    /* Load shader function with entry point */
    return LoadFunction(shaderDesc.entryPoint);
}

//TODO: this is untested!!!
bool MTShader::CompileBinary(id<MTLDevice> device, const ShaderDescriptor& shaderDesc)
{
    /* Get source */
    dispatch_data_t dispatchData = nil;
    NSData* source = nullptr;

    if (shaderDesc.source != nullptr)
    {
        if (shaderDesc.sourceType == ShaderSourceType::BinaryFile)
        {
            NSString* filePath = [[NSString alloc] initWithUTF8String:shaderDesc.source];
            source = [NSData dataWithContentsOfFile:filePath];
            dispatchData = dispatch_data_create([source bytes], [source length], nil, nil);
            [filePath release];
        }
        else if (shaderDesc.sourceSize > 0)
            dispatchData = dispatch_data_create(shaderDesc.source, shaderDesc.sourceSize, nil, nil);
    }

    if (dispatchData == nil)
        throw std::runtime_error("cannot compile Metal shader without source");

    /* Load shader library */
    ReleaseError();
    error_ = [NSError alloc];

    library_ = [device
        newLibraryWithData: reinterpret_cast<dispatch_data_t>(dispatchData)
        error:              &error_
    ];

    if (source != nullptr)
        [source release];

    [dispatchData release];

    /* Load shader function with entry point */
    return LoadFunction(shaderDesc.entryPoint);
}

void MTShader::ReleaseError()
{
    if (error_ != nullptr)
    {
        [error_ release];
        error_ = nullptr;
    }
}

bool MTShader::LoadFunction(const char* entryPoint)
{
    bool result = false;

    if (library_)
    {
        NSString* entryPointStr = ToNSString(entryPoint);

        /* Load shader function with entry point name */
        native_ = [library_ newFunctionWithName:entryPointStr];
        if (native_)
        {
            ReleaseError();
            result = true;
        }

        [entryPointStr release];
    }

    return result;
}


} // /namespace LLGL



// ================================================================================
