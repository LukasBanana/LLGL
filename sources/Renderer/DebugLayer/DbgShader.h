/*
 * DbgShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_SHADER_H
#define LLGL_DBG_SHADER_H


#include <LLGL/Shader.h>
#include <LLGL/RenderingDebugger.h>


namespace LLGL
{


class DbgShader : public Shader
{

    public:

        DbgShader(Shader& instance, const ShaderType type, RenderingDebugger* debugger);

        bool HasErrors() const override;

        std::string Disassemble(int flags = 0) override;

        std::string QueryInfoLog() override;

        inline bool IsCompiled() const
        {
            return !instance.HasErrors();
        }

        Shader& instance;

    private:

        RenderingDebugger* debugger_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
