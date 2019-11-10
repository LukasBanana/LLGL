/*
 * DbgShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_SHADER_PROGRAM_H
#define LLGL_DBG_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/RenderingDebugger.h>
#include <vector>
#include <string>


namespace LLGL
{


class DbgShader;

class DbgShaderProgram final : public ShaderProgram
{

    public:

        bool HasErrors() const override;
        std::string GetReport() const override;

        bool Reflect(ShaderReflection& reflection) const override;
        UniformLocation FindUniformLocation(const char* name) const override;

    public:

        struct VertexLayout
        {
            std::vector<VertexAttribute>    attributes;
            bool                            bound       = false;
        };

    public:

        DbgShaderProgram(
            ShaderProgram&                  instance,
            RenderingDebugger*              debugger,
            const ShaderProgramDescriptor&  desc
        );

        // Returns the vertex layout meta data.
        inline const VertexLayout& GetVertexLayout() const
        {
            return vertexLayout_;
        }

        // Returns true if this shader program contains a fragment shader.
        inline bool HasFragmentShader() const
        {
            return hasFragmentShader_;
        }

        // Returns the name of the vertex ID if the shader program makes use of the SV_VertexID, gl_VertexID, or gl_VertexIndex semantics. Returns null otherwise.
        const char* GetVertexID() const;

        // Returns the name of the instance ID if the shader program makes use of the SV_InstanceID, gl_InstanceID, or gl_InstanceIndex semantics. Returns null otherwise.
        const char* GetInstanceID() const;

    public:

        ShaderProgram& instance;

    private:

        void ValidateShaderAttachment(Shader* shader, const ShaderType type);
        void QueryInstanceAndVertexIDs();

    private:

        RenderingDebugger*  debugger_               = nullptr;
        bool                hasFragmentShader_      = false;

        VertexLayout        vertexLayout_;

        std::string         vertexID_;
        std::string         instanceID_;

};


} // /namespace LLGL


#endif



// ================================================================================
