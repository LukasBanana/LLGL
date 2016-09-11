/*
 * ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_SHADER_PROGRAM_H__
#define __LLGL_SHADER_PROGRAM_H__


#include "Export.h"
#include "Shader.h"
#include "VertexAttribute.h"
#include "ConstantBuffer.h"
#include "StorageBuffer.h"
#include "ShaderUniform.h"
#include <string>
#include <vector>


namespace LLGL
{


//! Shader program interface.
class LLGL_EXPORT ShaderProgram
{

    public:

        virtual ~ShaderProgram()
        {
        }

        /**
        \brief Attaches the specified shader to this shader program.
        \param[in] shader Specifies the shader which is to be attached to this shader program.
        Each shader type can only be added once for each shader program.
        \remarks This must be called, before "LinkShaders" is called.
        \throws std::invalid_argument If a shader is attached to this shader program, which is not allow in the current state.
        This will happend if a different shader of the same type has already been attached to this shader program.
        \see Shader::GetType
        */
        virtual void AttachShader(Shader& shader) = 0;

        /**
        \brief Links all attached shaders to the final shader program.
        \return True on success, otherwise "QueryInfoLog" can be used to query the reason for failure.
        \remarks Each attached shader must be compiled first!
        \see QueryInfoLog
        */
        virtual bool LinkShaders() = 0;

        //! Returns the information log after the shader linkage.
        virtual std::string QueryInfoLog() = 0;

        //! Returns a list of vertex attributes, which describe all vertex attributes within this shader program.
        virtual std::vector<VertexAttribute> QueryVertexAttributes() const = 0;

        //! Returns a list of constant buffer descriptors, which describe all constant buffers (also "Uniform Buffer Object") within this shader program.
        virtual std::vector<ConstantBufferDescriptor> QueryConstantBuffers() const = 0;

        //! Returns a list of storage buffer descriptors, which describe all storage buffers (also "Shader Storage Buffer Object" or "Read/Write Buffer") within this shader program.
        virtual std::vector<StorageBufferDescriptor> QueryStorageBuffers() const = 0;

        /**
        \brief Returns a list of uniform descriptors, which describe all uniforms within this shader program.
        \remarks Shader uniforms are only supported in OpenGL 2.0+.
        */
        virtual std::vector<UniformDescriptor> QueryUniforms() const = 0;

        /**
        \brief Binds the specified vertex attributes to this shader program.
        \param[in] vertexAttribs Specifies the vertex attributes.
        \remarks This is only required for a shader program, which has an attached vertex shader.
        Moreover, this can only be called after shader compilation but before shader program linking!
        \see AttachShader(VertexShader&)
        \see Shader::Compile
        \see LinkShaders
        \throws std::invalid_argument If the name of an vertex attribute is invalid or the maximal number of available vertex attributes is exceeded.
        */
        virtual void BindVertexAttributes(const std::vector<VertexAttribute>& vertexAttribs) = 0;

        /**
        \brief Binds the specified constant buffer to this shader.
        \param[in] name Specifies the name of the constant buffer within this shader.
        \param[in] bindingIndex Specifies the binding index. This index must match the index which will be used for "RenderContext::BindConstantBuffer".
        \see QueryConstantBuffers
        \see RenderContext::BindConstantBuffer
        */
        virtual void BindConstantBuffer(const std::string& name, unsigned int bindingIndex) = 0;

        /**
        \brief Binds the specified storage buffer to this shader.
        \param[in] name Specifies the name of the storage buffer within this shader.
        \param[in] bindingIndex Specifies the binding index. This index must match the index which will be used for "RenderContext::BindStorageBuffer".
        \see RenderContext::BindStorageBuffer
        */
        virtual void BindStorageBuffer(const std::string& name, unsigned int bindingIndex) = 0;

        /**
        \brief Locks the shader uniform handler.
        \return Pointer to the shader uniform handler or null if the render system does not support individual shader uniforms.
        \remarks This must be called to set individual shader uniforms.
        \code
        auto uniform = shaderProgram->LockShaderUniform();
        if (uniform)
        {
            uniform->SetUniform("mySampler1", 0);
            uniform->SetUniform("mySampler2", 1);
            uniform->SetUniform("projection", myProjectionMatrix);
        }
        shaderProgram->UnlockShaderUniform();
        \endcode
        \note Only a shader program from an OpenGL render system will return a non-null pointer!
        */
        virtual ShaderUniform* LockShaderUniform() = 0;

        /**
        \brief Unlocks the shader uniform handler.
        \see LockShaderUniform
        */
        virtual void UnlockShaderUniform() = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
