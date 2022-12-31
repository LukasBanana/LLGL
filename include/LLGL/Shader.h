/*
 * Shader.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_H
#define LLGL_SHADER_H


#include <LLGL/RenderSystemChild.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/ShaderReflection.h>
#include <LLGL/Report.h>


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
        \brief Returns a pointer to the report or null if there is none.
        \remarks If there is a report, it might contain warnings and/or errors from shader compilation process.
        \see Report
        */
        virtual const Report* GetReport() const = 0;

        /**
        \brief Returns a reflection of the shader pipeline layout with all required resources for this shader.
        \param[out] reflection Specifies the output shader reflection. If the function returns \c false, the content of this parameter is <b>undefined</b>!
        \remarks The list of resources in the reflection output is always sorted by the following attributes (lower number means higher priority for sorting):
        -# Resource type in ascending order (see BindingDescriptor::type).
        -# Binding flags in ascending order (see BindingDescriptor::bindFlags).
        -# Binding slot in ascending order (see BindingDescriptor::slot).
        \remarks Here is an example of such a sorted list (pseudocode):
        \code{.txt}
        resources[0] = { type: ResourceType::Buffer,  bindFlags: BindFlags::ConstantBuffer, slot: 0 }
        resources[1] = { type: ResourceType::Buffer,  bindFlags: BindFlags::ConstantBuffer, slot: 2 }
        resources[2] = { type: ResourceType::Texture, bindFlags: BindFlags::Sampled,        slot: 0 }
        resources[3] = { type: ResourceType::Texture, bindFlags: BindFlags::Sampled,        slot: 1 }
        resources[4] = { type: ResourceType::Texture, bindFlags: BindFlags::Sampled,        slot: 2 }
        resources[5] = { type: ResourceType::Sampler, bindFlags: 0,                         slot: 2 }
        \endcode
        \remarks The \c instanceDivisor and \c offset members of the vertex attributes are ignored by this function.
        \return True, if the reflection was successful. Otherwise, the shader reflection failed and the content of the output parameter \c reflection is undefined.
        \see ShaderReflection::resources
        \see ShaderReflection::vertexAttributes
        \see VertexAttribute::instanceDivisor
        \see VertexAttribute::offset
        */
        virtual bool Reflect(ShaderReflection& reflection) const = 0;

    public:

        /**
        \brief Returns the location of a single shader uniform by its name.
        \returns Uniform location of the specified uniform, or -1 if there is no such uniform in the shader program.
        \remarks This is a helper function when only one or a few number of uniform locations are meant to be determined.
        If more uniforms are involved, use the Reflect function.
        \remarks Default implementation always returns Constants::invalidLocation.
        \see Reflect
        \note Only supported with: OpenGL.
        */
        virtual UniformLocation FindUniformLocation(const char* name) const;

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
