/*
 * ShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_PROGRAM_H
#define LLGL_SHADER_PROGRAM_H


#include "Export.h"
#include "Shader.h"
#include "VertexFormat.h"
#include "StreamOutputFormat.h"
#include "BufferFlags.h"
#include "ShaderUniform.h"
#include <string>
#include <vector>


namespace LLGL
{


//! Shader program interface.
class LLGL_EXPORT ShaderProgram
{

    public:

        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator = (const ShaderProgram&) = delete;

        virtual ~ShaderProgram()
        {
        }

        /**
        \brief Attaches the specified shader to this shader program.
        \param[in] shader Specifies the shader which is to be attached to this shader program.
        Each shader type can only be added once for each shader program.
        \remarks This must be called, before "LinkShaders" is called.
        \throws std::invalid_argument If a shader is attached to this shader program, which is not allowed in the current state.
        This will happend if a different shader of the same type has already been attached to this shader program for instance.
        \see Shader::GetType
        */
        virtual void AttachShader(Shader& shader) = 0;

        /**
        \brief Detaches all shaders from this shader program.
        \remarks After this call, the link status will be invalid, and the shader program must be linked again.
        \see LinkShaders
        */
        virtual void DetachAll() = 0;

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

        //! Returns a list of stream-output attributes, which describes all stream-output attributes within this shader program.
        virtual std::vector<StreamOutputAttribute> QueryStreamOutputAttributes() const = 0;

        /**
        \brief Returns a list of constant buffer view descriptors, which describe all constant buffers within this shader program.
        \remarks Also called "Uniform Buffer Object".
        */
        virtual std::vector<ConstantBufferViewDescriptor> QueryConstantBuffers() const = 0;

        /**
        \brief Returns a list of storage buffer view descriptors, which describe all storage buffers within this shader program.
        \remarks Also called "Shader Storage Buffer Object" or "Read/Write Buffer".
        */
        virtual std::vector<StorageBufferViewDescriptor> QueryStorageBuffers() const = 0;

        /**
        \brief Returns a list of uniform descriptors, which describe all uniforms within this shader program.
        \remarks Shader uniforms are only supported in OpenGL 2.0+.
        */
        virtual std::vector<UniformDescriptor> QueryUniforms() const = 0;

        /**
        \brief Builds the input layout with the specified vertex format for this shader program.
        \param[in] vertexFormat Specifies the input vertex format.
        \remarks Can only be used for a shader program which has a successfully compiled vertex shader attached.
        If this is called after the shader program has been linked, the shader program might be re-linked again.
        \see AttachShader(VertexShader&)
        \see Shader::Compile
        \see LinkShaders
        \throws std::invalid_argument If the name of an vertex attribute is invalid or the maximal number of available vertex attributes is exceeded.
        */
        virtual void BuildInputLayout(const VertexFormat& vertexFormat) = 0;

        //TODO: add FragmentFormat structure to provide CPU side fragment binding for OpenGL
        //virtual void BuildOutputLayout(const FragmentFormat& fragmentFormat) = 0;

        /**
        \brief Binds the specified constant buffer to this shader.
        \param[in] name Specifies the name of the constant buffer within this shader.
        \param[in] bindingIndex Specifies the binding index. This index must match the index which will be used for "RenderContext::BindConstantBuffer".
        \remarks This function is only necessary if the binding index does not match the default binding index of the constant buffer within the shader.
        \see QueryConstantBuffers
        \see RenderContext::BindConstantBuffer
        */
        virtual void BindConstantBuffer(const std::string& name, unsigned int bindingIndex) = 0;

        /**
        \brief Binds the specified storage buffer to this shader.
        \param[in] name Specifies the name of the storage buffer within this shader.
        \param[in] bindingIndex Specifies the binding index. This index must match the index which will be used for "RenderContext::BindStorageBuffer".
        \remarks This function is only necessary if the binding index does not match the default binding index of the storage buffer within the shader.
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

        ShaderProgram() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
