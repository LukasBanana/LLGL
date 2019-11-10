/*
 * Shader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_H
#define LLGL_SHADER_H


#include "RenderSystemChild.h"
#include "ShaderFlags.h"


namespace LLGL
{


/**
\brief Shader interface.
\see RenderSystem::CreateShader
*/
class LLGL_EXPORT Shader : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Shader );

    public:

        /**
        \brief Returns true if this shader has any errors. Otherwise, the compilation was successful.
        \remarks If the compilation failed, this shader can not be used for a graphics or compute pipeline.
        However, the details about the failure can be queried by the GetReport function.
        \see GetReport
        */
        virtual bool HasErrors() const = 0;

        /**
        \brief Returns the report message after the shader compilation or an empty string if there is no report.
        \todo Change return value to std::unique_ptr<LLGL::Blob>
        \see ShaderProgram::GetReport
        */
        virtual std::string GetReport() const = 0;

        /**
        \brief Returns true if this is a post-tessellation vertex shader.
        \remarks This is only used for the Metal backend to determine whether a post-tessellation vertex shader is used in conjunction with a compute kernel.
        \remarks Default implementation always returns false.
        \note Only supported with: Metal.
        */
        virtual bool IsPostTessellationVertex() const;

        /**
        \brief Returns the shader stage bitmask for this shader object.
        \see StageFlags
        */
        long GetStageFlags() const;

        //! Returns the type of this shader.
        inline ShaderType GetType() const
        {
            return type_;
        }

    protected:

        Shader(const ShaderType type);

    private:

        ShaderType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
