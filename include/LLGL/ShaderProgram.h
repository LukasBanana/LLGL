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
#include "ShaderUniform.h"


namespace LLGL
{


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
        \brief Binds the specified constant buffer to this shader.
        \param[in] name Specifies the name of the constant buffer within this shader.
        \param[in] bindingIndex Specifies the binding index. This index must match the index which will be used for "RenderContext::BindConstantBuffer".
        \remarks This function is only necessary if the binding index does not match the default binding index of the constant buffer within the shader.
        \see QueryConstantBuffers
        \see RenderContext::BindConstantBuffer
        \todo Replace this by PipelineLayout
        */
        virtual void BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex) = 0;

        /**
        \brief Binds the specified storage buffer to this shader.
        \param[in] name Specifies the name of the storage buffer within this shader.
        \param[in] bindingIndex Specifies the binding index. This index must match the index which will be used for "RenderContext::BindStorageBuffer".
        \remarks This function is only necessary if the binding index does not match the default binding index of the storage buffer within the shader.
        \see RenderContext::BindStorageBuffer
        \todo Replace this by PipelineLayout
        */
        virtual void BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex) = 0;

        /**
        \brief Locks the shader uniform handler.
        \return Pointer to the shader uniform handler or null if the render system does not support individual shader uniforms.
        \remarks This must be called to set individual shader uniforms.
        \code
        if (auto myUniformHandler = myShaderProgram->LockShaderUniform()) {
            myUniformHandler->SetUniform1i("mySampler1", 0);
            myUniformHandler->SetUniform1i("mySampler2", 1);
            myUniformHandler->SetUniform4x4fv("projection", &myProjectionMatrix[0]);
            myShaderProgram->UnlockShaderUniform();
        }
        \endcode
        \note Only supported with: OpenGL.
        \see UnlockShaderUniform
        */
        virtual ShaderUniform* LockShaderUniform() = 0;

        /**
        \brief Unlocks the shader uniform handler.
        \see LockShaderUniform
        */
        virtual void UnlockShaderUniform() = 0;

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
