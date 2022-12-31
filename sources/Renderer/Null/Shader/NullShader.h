/*
 * NullShader.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NULL_SHADER_H
#define LLGL_NULL_SHADER_H


#include <LLGL/Shader.h>
#include <string>


namespace LLGL
{


class NullShader final : public Shader
{

    public:

        void SetName(const char* name) override;
        const Report* GetReport() const override;
        bool Reflect(ShaderReflection& reflection) const override;

    public:

        NullShader(const ShaderDescriptor& desc);

    public:

        const ShaderDescriptor desc;

    public:

        std::string label_;

};


} // /namespace LLGL


#endif



// ================================================================================
