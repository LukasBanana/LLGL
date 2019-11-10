/*
 * DbgShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_SHADER_H
#define LLGL_DBG_SHADER_H


#include <LLGL/Shader.h>
#include <LLGL/RenderingDebugger.h>
#include <string>


namespace LLGL
{


class DbgShader final : public Shader
{

    public:

        void SetName(const char* name) override;

        bool HasErrors() const override;

        std::string GetReport() const override;

        bool IsPostTessellationVertex() const override;

    public:

        DbgShader(Shader& instance, const ShaderDescriptor& desc);

        inline bool IsCompiled() const
        {
            return !instance.HasErrors();
        }

    public:

        Shader&                 instance;
        const ShaderDescriptor  desc;
        std::string             label;

};


} // /namespace LLGL


#endif



// ================================================================================
