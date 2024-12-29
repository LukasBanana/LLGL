/*
 * GLPipelineState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_PIPELINE_STATE_H
#define LLGL_GL_PIPELINE_STATE_H


#include "../OpenGL.h"
#include "../Shader/GLShaderBindingLayout.h"
#include "../Shader/GLShaderPipeline.h"
#include "../Shader/GLShader.h"
#include "../Shader/GLShaderBufferInterfaceMap.h"
#include <LLGL/Report.h>
#include <LLGL/PipelineState.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <memory>
#include <unordered_map>


namespace LLGL
{


class PipelineLayout;
class PipelineCache;
class GLStateManager;
class GLShaderProgram;

// GL uniform location with size and type information.
struct GLUniformLocation
{
    UniformType type;
    GLint       location;
    GLsizei     count;
    GLuint      wordSize; // Size in words (32-bit values)
};

// Base class for OpenGL PSOs.
class GLPipelineState : public PipelineState
{

    public:

        GLPipelineState(
            bool                        isGraphicsPSO,
            const PipelineLayout*       pipelineLayout,
            PipelineCache*              pipelineCache,
            const ArrayView<Shader*>&   shaders
        );
        ~GLPipelineState();

        const Report* GetReport() const override;

        // Binds this pipeline state with the specified GL state manager.
        virtual void Bind(GLStateManager& stateMngr);

        // Returns true if this is a graphics PSO.
        inline bool IsGraphicsPSO() const
        {
            return isGraphicsPSO_;
        }

        // Returns the pipeline layout this PSO was created with. May also be null.
        inline const GLPipelineLayout* GetPipelineLayout() const
        {
            return pipelineLayout_;
        }

        // Returns the shader pipeline used for this PSO.
        inline const GLShaderPipeline* GetShaderPipeline() const
        {
            return shaderPipelines_[GLShader::PermutationDefault].get();
        }

        // Returns the list of uniforms that maps from index of 'PipelineLayoutDescriptor::uniforms[]' to GL uniform location.
        inline const std::vector<GLUniformLocation>& GetUniformMap() const
        {
            return uniformMap_;
        }

        // Returns the interface map for SSBOs, sampler buffers, and image buffers.
        inline const GLShaderBufferInterfaceMap* GetBufferInterfaceMap() const
        {
            return &bufferInterfaceMap_;
        }

        // Returns the GL bitfield of the memory barriers the pipeline layout of this PSO was created with. See GLPipelineLayout::GetBarriersBitfield().
        inline GLbitfield GetBarriersBitfield() const
        {
            return barriers_;
        }

    protected:

        // Returns a mutable reference to the PSO report.
        inline Report& GetMutableReport()
        {
            return report_;
        }

    private:

        struct GLActiveUniform
        {
            GLint   size;
            GLenum  type;
        };

        // Maps a name to its active GL uniform.
        using GLNameToUniformMap = std::unordered_map<std::string, GLActiveUniform>;

    private:

        // Builds the index-to-uniform map.
        void BuildUniformMap(GLShader::Permutation permutation, const std::vector<UniformDescriptor>& uniforms);

        // Builds the container that maps a name to the index of its active GL uniform.
        void BuildNameToActiveUniformMap(GLuint program, GLNameToUniformMap& outNameToUniformMap);

        // Builds the specified uniform location.
        void BuildUniformLocation(
            GLuint                      program,
            GLUniformLocation&          outUniform,
            const UniformDescriptor&    inUniform,
            const GLNameToUniformMap&   nameToUniformMap
        );

    private:

        const bool                      isGraphicsPSO_                                  = false;
        GLbitfield                      barriers_                                       = 0;
        const GLPipelineLayout*         pipelineLayout_                                 = nullptr;
        GLShaderPipelineSPtr            shaderPipelines_[GLShader::PermutationCount];
        GLShaderBindingLayoutSPtr       shaderBindingLayout_;
        GLShaderBufferInterfaceMap      bufferInterfaceMap_;
        std::vector<GLUniformLocation>  uniformMap_;
        Report                          report_;

};


} // /namespace LLGL


#endif



// ================================================================================
