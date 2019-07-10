/*
 * ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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

    public:

        /**
        \brief Returns true if this shader program has any errors. Otherwise, the linking was successful.
        \remarks If the linking failed, this shader program can not be used for a graphics or compute pipeline.
        However, the details about the failure can be queried by the QueryInfoLog function.
        \see QueryInfoLog
        */
        virtual bool HasErrors() const = 0;

        //! Returns the information log after the shader linkage.
        virtual std::string QueryInfoLog() = 0;

        /**
        \brief Returns a descriptor of the shader pipeline layout with all required shader resources.
        \remarks The list of resource views in the output descriptor (i.e. 'resourceViews' attribute) is always sorted in the following manner:
        First sorting criterion is the resource type (in ascending order),
        second sorting criterion is the binding flags (in ascending order),
        third sorting criterion is the binding slot (in ascending order).
        Here is an example of such a sorted list (pseudocode):
        \code{.txt}
        resourceViews[0] = { type: ResourceType::Buffer,  bindFlags: BindFlags::ConstantBuffer, slot: 0 }
        resourceViews[1] = { type: ResourceType::Buffer,  bindFlags: BindFlags::ConstantBuffer, slot: 2 }
        resourceViews[2] = { type: ResourceType::Texture, bindFlags: BindFlags::SampleBuffer,   slot: 0 }
        resourceViews[3] = { type: ResourceType::Texture, bindFlags: BindFlags::SampleBuffer,   slot: 1 }
        resourceViews[4] = { type: ResourceType::Texture, bindFlags: BindFlags::SampleBuffer,   slot: 2 }
        resourceViews[5] = { type: ResourceType::Sampler, bindFlags: 0,                         slot: 2 }
        \endcode
        The \c instanceDivisor and \c offset members of the vertex attributes are ignored by this function.
        \see ShaderReflectionDescriptor::resourceViews
        \see ShaderReflectionDescriptor::vertexAttributes
        \see VertexAttribute::instanceDivisor
        \see VertexAttribute::offset
        \throws std::runtime_error If shader reflection failed.
        */
        virtual ShaderReflectionDescriptor QueryReflectionDesc() const = 0;

        /**
        \brief Returns the location of a single shader uniform by its name.
        \returns Uniform location of the specified uniform, or -1 if there is no such uniform in the shader program.
        \remarks This is a helper function when only one or a few number of uniform locations are meant to be determined.
        If more uniforms are involved, use the QueryReflectionDesc function.
        \see QueryReflectionDesc
        \note Only supported with: OpenGL, Vulkan, Direct3D 12.
        */
        virtual UniformLocation QueryUniformLocation(const char* name) const = 0;

        /**
        \brief Sets the work group size of a compute shader, i.e. the number of threads per thread-group. By default (1, 1, 1).
        \param[in] workGroupSize Specifies the number of threads per thread-group in X, Y, and Z direction.
        Each component must be greater than zero.
        \return True, if the work group size can be dynamically set and the values are valid.
        Otherwise, the work group size must be specified within the shader code or the values are invalid.
        If the return value is false, the function call has no effect.
        \remarks Only the Metal backend supports dispatch compute kernels with dynamic work group sizes.
        For all other renderers, the work group size must be specified within the shader code:
        - For GLSL: <code>layout(local_size_x = X, local_size_y = Y, local_size_z = Z)</code>
        - For HLSL: <code>[numthreads(X, Y, Z)]</code>
        \note Only supported with: Metal.
        */
        virtual bool SetWorkGroupSize(const Extent3D& workGroupSize) = 0;

        /**
        \brief Retrieves the work group size of a compute shader, i.e. the number of threads per thread-group.
        \param[out] workGroupSize Specifies the number of threads per thread-group in X, Y, and Z direction.
        This output parameter is not modified, if the function returns false.
        \return True, if the work group size could be determined.
        Otherwise, the rendering API does not support shader reflection to query the work group size,
        or the shader program does not contain a compute shader.
        */
        virtual bool GetWorkGroupSize(Extent3D& workGroupSize) const = 0;

    protected:

        //! Linker error codes for internal error checking.
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
        \brief Sorts the resource views of the specified shader reflection descriptor as described in the QueryReflectionDesc function.
        \see QueryReflectionDesc
        */
        static void FinalizeShaderReflection(ShaderReflectionDescriptor& reflectionDesc);

        //! Returns a string representation for the specified shader linker error, or null if the no error is entered (i.e. LinkError::NoError).
        static const char* LinkErrorToString(const LinkError errorCode);

};


} // /namespace LLGL


#endif



// ================================================================================
