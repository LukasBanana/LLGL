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
#include "../Shader/GLShaderPipeline.h"
#include "../../../Core/BasicReport.h"
#include <LLGL/PipelineState.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/Container/ArrayView.h>
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
            bool                        isGraphicsPSO,
            const PipelineLayout*       pipelineLayout,
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

        // Returns the shader pipeline used for this PSO.
        inline const GLShaderPipeline* GetShaderPipeline() const
        {
            return shaderPipeline_.get();
        }

    private:

        const bool                  isGraphicsPSO_          = false;
        GLShaderPipelineSPtr        shaderPipeline_         = nullptr;
        GLShaderBindingLayoutSPtr   shaderBindingLayout_;
        BasicReport                 report_;

};


} // /namespace LLGL


#endif



// ================================================================================
