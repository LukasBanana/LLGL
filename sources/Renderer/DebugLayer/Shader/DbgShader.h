/*
 * DbgShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

        #include <LLGL/Backend/Shader.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        DbgShader(Shader& instance, const ShaderDescriptor& desc);

        // Returns the name of the vertex ID if the shader program makes use of the SV_VertexID, gl_VertexID, or gl_VertexIndex semantics. Returns null otherwise.
        const char* GetVertexID() const;

        // Returns the name of the instance ID if the shader program makes use of the SV_InstanceID, gl_InstanceID, or gl_InstanceIndex semantics. Returns null otherwise.
        const char* GetInstanceID() const;

        // Returns true if this shader has no errors.
        bool IsCompiled() const;

        // Returns true if this shader has any output attributes.
        inline bool HasAnyOutputAttributes() const
        {
            return hasAnyOutputAttribs_;
        }

    public:

        Shader&                 instance;
        const ShaderDescriptor  desc;
        std::string             label;

    private:

        void CacheShaderReflection();

    private:

        std::string             vertexID_;
        std::string             instanceID_;
        bool                    hasAnyOutputAttribs_    = false;

};


} // /namespace LLGL


#endif



// ================================================================================
