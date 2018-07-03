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

        MTShader(id<MTLDevice> device, const ShaderDescriptor& desc);
        ~MTShader();
    
        bool HasErrors() const override;

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

        bool CompileSource(id<MTLDevice> device, const ShaderDescriptor& shaderDesc);
        void ReleaseError();

        id<MTLLibrary>  library_    = nil;
        id<MTLFunction> native_     = nil;

        NSError*        error_      = nullptr;
        bool            hasErrors_  = false;

};


} // /namespace LLGL


#endif



// ================================================================================
