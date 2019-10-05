/*
 * GLPipelineState.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_PIPELINE_STATE_H
#define LLGL_GL_PIPELINE_STATE_H


#include "../OpenGL.h"
#include "../Shader/GLShaderBindingLayout.h"
#include <LLGL/PipelineState.h>
#include <LLGL/RenderSystemFlags.h>
#include <memory>


namespace LLGL
{


class PipelineLayout;
class ShaderProgram;
class GLStateManager;
class GLShaderProgram;

class GLPipelineState : public PipelineState
{

    public:

        GLPipelineState(
            bool                    isGraphicsPSO,
            const PipelineLayout*   pipelineLayout,
            const ShaderProgram*    shaderProgram
        );
        ~GLPipelineState();

        // Binds this pipeline state with the specified GL state manager.
        virtual void Bind(GLStateManager& stateMngr);

        // Returns true if this is a graphics PSO.
        inline bool IsGraphicsPSO() const
        {
            return isGraphicsPSO_;
        }

        // Returns the shader program used for this graphics pipeline.
        inline const GLShaderProgram* GetShaderProgram() const
        {
            return shaderProgram_;
        }

    private:

        const bool                  isGraphicsPSO_          = false;
        const GLShaderProgram*      shaderProgram_          = nullptr;
        GLShaderBindingLayoutSPtr   shaderBindingLayout_;

};


} // /namespace LLGL


#endif



// ================================================================================
