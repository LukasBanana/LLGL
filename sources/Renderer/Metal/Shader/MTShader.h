/*
 * MTShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_SHADER_H
#define LLGL_MT_SHADER_H


#import <Metal/Metal.h>

#include <LLGL/Shader.h>


namespace LLGL
{


class MTShader : public Shader
{

    public:

        MTShader(id<MTLDevice> device, const ShaderType type);
        ~MTShader();

        bool Compile(const std::string& sourceCode, const ShaderDescriptor& shaderDesc = {}) override;

        bool LoadBinary(std::vector<char>&& binaryCode, const ShaderDescriptor& shaderDesc = {}) override;

        std::string Disassemble(int flags = 0) override;

        std::string QueryInfoLog() override;

        // Returns the native MTLFunction object.
        inline id<MTLFunction> GetNative() const
        {
            return native_;
        }

        // Returns true if the shader has an error report.
        bool HasError() const
        {
            return (error_ != nullptr);
        }

    private:
    
        void ReleaseError();

        id<MTLDevice>   device_     = nil;
        id<MTLLibrary>  library_    = nil;
        id<MTLFunction> native_     = nil;
    
        NSError*        error_      = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
