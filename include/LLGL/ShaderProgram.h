/*
 * ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_PROGRAM_H
#define LLGL_SHADER_PROGRAM_H


#include "RenderSystemChild.h"
#include "ShaderProgramFlags.h"


namespace LLGL
{


struct Extent3D;

/**
\brief Shader program interface.
\remarks A shader program combines multiple instances of the Shader class to be used in a complete shader pipeline.
\see RenderSystem::CreateShaderProgram
*/
class LLGL_EXPORT ShaderProgram : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::ShaderProgram );

    public:

        /**
        \brief Returns true if this shader program has any errors. Otherwise, the linking was successful.
        \remarks If the linking failed, this shader program can not be used for a graphics or compute pipeline.
        However, the details about the failure can be queried by the GetReport function.
        \see GetReport
        */
        virtual bool HasErrors() const = 0;

        /**
        \brief Returns the report message after the shader linkage or an empty string if there is no report.
        \see Shader::GetReport
        */
        virtual std::string GetReport() const = 0;

        /**
        \brief Returns a reflection of the shader pipeline layout with all required shader resources.
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

        /**
        \brief Returns the location of a single shader uniform by its name.
        \returns Uniform location of the specified uniform, or -1 if there is no such uniform in the shader program.
        \remarks This is a helper function when only one or a few number of uniform locations are meant to be determined.
        If more uniforms are involved, use the Reflect function.
        \see Reflect
        \note Only supported with: OpenGL.
        */
        virtual UniformLocation FindUniformLocation(const char* name) const = 0;

    protected:

        /**
        \brief Linker error codes for internal error checking.
        \todo Turn this into a global common enumeration for error codes.
        */
        enum class LinkError
        {
            NoError,
            InvalidComposition,
            InvalidByteCode,
            TooManyAttachments,
            IncompleteAttachments,
        };

    protected:

        /**
        \brief Validates the composition of the specified shader attachments.
        \param[in] shaders Array of Shader objects that belong to this shader program. Null pointers within the array are ignored.
        \param[in] numShaders Specifies the number of entries in the array 'shaders'. This must not be larger than the number of entries in the 'shaders' array.
        \return True if the shader composition is valid, otherwise false.
        \remarks For example, a composition of a compute shader and a fragment shader is invalid,
        but a composition of a vertex shader and a fragment shader is valid.
        */
        static bool ValidateShaderComposition(Shader* const * shaders, std::size_t numShaders);

        /**
        \brief Clears all members in the specified shader reflection.
        \remarks Use this to start reflecting a shader program as shown in the following example:
        \code
        bool MyShaderProgram::Reflect(ShaderReflection& reflection)
        {
            ShaderProgram::ClearShaderReflection(reflection);
            auto succeded = // reflection code here ...
            FinalizeShaderReflection(reflection);
            return succeded;
        }
        \endcode
        \see Reflect
        \see FinalizeShaderReflection
        */
        static void ClearShaderReflection(ShaderReflection& reflection);

        /**
        \brief Sorts the resource views of the specified shader reflection descriptor as described in the Reflect function.
        \see Reflect
        \see ClearShaderReflection
        */
        static void FinalizeShaderReflection(ShaderReflection& reflection);

        //! Returns a string representation for the specified shader linker error, or null if the no error is entered (i.e. LinkError::NoError).
        static const char* LinkErrorToString(const LinkError errorCode);

};


} // /namespace LLGL


#endif



// ================================================================================
