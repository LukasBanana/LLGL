/*
 * DbgShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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

        struct VertexLayout
        {
            std::vector<VertexAttribute>    attributes;
            bool                            bound       = false;
        };

        DbgShaderProgram(
            ShaderProgram&                  instance,
            RenderingDebugger*              debugger,
            const ShaderProgramDescriptor&  desc,
            const RenderingCapabilities&    caps
        );

        bool HasErrors() const override;

        std::string QueryInfoLog() override;

        ShaderReflection QueryReflection() const override;
        UniformLocation QueryUniformLocation(const char* name) const override;

        bool SetWorkGroupSize(const Extent3D& workGroupSize) override;
        bool GetWorkGroupSize(Extent3D& workGroupSize) const override;

    public:

        inline const VertexLayout& GetVertexLayout() const
        {
            return vertexLayout_;
        }

        // Returns the name of the vertex ID if the shader program makes use of the SV_VertexID, gl_VertexID, or gl_VertexIndex semantics. Returns null otherwise.
        const char* GetVertexID() const;

        // Returns the name of the instance ID if the shader program makes use of the SV_InstanceID, gl_InstanceID, or gl_InstanceIndex semantics. Returns null otherwise.
        const char* GetInstanceID() const;

    public:

        ShaderProgram& instance;

    private:

        void ValidateShaderAttachment(Shader* shader);
        void ValidateShaderComposition();
        void QueryInstanceAndVertexIDs(const RenderingCapabilities& caps);

    private:

        RenderingDebugger*      debugger_               = nullptr;
        int                     shaderAttachmentMask_   = 0;

        std::vector<ShaderType> shaderTypes_;
        VertexLayout            vertexLayout_;

        std::string             vertexID_;
        std::string             instanceID_;

};


} // /namespace LLGL


#endif



// ================================================================================
