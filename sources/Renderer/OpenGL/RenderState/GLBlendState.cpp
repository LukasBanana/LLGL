/*
 * GLBlendState.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBlendState.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../GLCore.h"
#include "../GLTypes.h"
#include "../GLProfile.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/HelperMacros.h"
#include "../Texture/GLRenderTarget.h"
#include "GLStateManager.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


static void Convert(GLfloat (&dst)[4], const ColorRGBAf& src)
{
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
}

GLBlendState::GLBlendState(const BlendDescriptor& desc, std::uint32_t numColorAttachments)
{
    Convert(blendColor_, desc.blendFactor);

    blendColorDynamic_      = desc.blendFactorDynamic;
    blendColorEnabled_      = IsStaticBlendFactorEnabled(desc);
    sampleAlphaToCoverage_  = desc.alphaToCoverageEnabled;
    sampleMask_             = desc.sampleMask;

    #ifdef LLGL_OPENGL
    if (desc.logicOp != LogicOp::Disabled)
    {
        logicOpEnabled_ = true;
        logicOp_        = GLTypes::Map(desc.logicOp);
    }
    #endif

    if (desc.independentBlendEnabled)
    {
        GLDrawBufferState::Convert(drawBuffers_[0], desc.targets[0]);
        numDrawBuffers_ = 1;
    }
    else
    {
        numDrawBuffers_ = numColorAttachments;
        for (std::uint32_t i = 0; i < numColorAttachments; ++i)
            GLDrawBufferState::Convert(drawBuffers_[i], desc.targets[i]);
    }

    #if 0//TODO
    if (multiSampleEnabled_)
        stateMngr.SetSampleMask(sampleMask_);
    #endif
}

void GLBlendState::Bind(GLStateManager& stateMngr)
{
    /* Set blend factor */
    if (blendColorEnabled_)
        stateMngr.SetBlendColor(blendColor_);

    stateMngr.Set(GLState::SAMPLE_ALPHA_TO_COVERAGE, sampleAlphaToCoverage_);

    #ifdef LLGL_OPENGL

    if (logicOpEnabled_)
    {
        /* Enable logic pixel operation */
        stateMngr.Enable(GLState::COLOR_LOGIC_OP);
        stateMngr.SetLogicOp(logicOp_);

        /* Bind only color masks for all draw buffers */
        BindDrawBufferColorMasks(stateMngr);
    }
    else
    {
        /* Disable logic pixel operation */
        stateMngr.Disable(GLState::COLOR_LOGIC_OP);

        /* Bind blend states for all draw buffers */
        BindDrawBufferStates(stateMngr);
    }

    #else // LLGL_OPENGL

    /* Bind blend states for all draw buffers */
    BindDrawBufferStates(stateMngr);

    #endif // /LLGL_OPENGL
}

void GLBlendState::BindColorMaskOnly(GLStateManager& stateMngr)
{
    BindDrawBufferColorMasks(stateMngr);
}

int GLBlendState::CompareSWO(const GLBlendState& rhs) const
{
    const auto& lhs = *this;

    LLGL_COMPARE_MEMBER_SWO     ( blendColor_[0]         );
    LLGL_COMPARE_MEMBER_SWO     ( blendColor_[1]         );
    LLGL_COMPARE_MEMBER_SWO     ( blendColor_[2]         );
    LLGL_COMPARE_MEMBER_SWO     ( blendColor_[3]         );
    LLGL_COMPARE_MEMBER_SWO     ( sampleAlphaToCoverage_ );
    //LLGL_COMPARE_MEMBER_SWO     ( sampleMask_            );
    #ifdef LLGL_OPENGL
    LLGL_COMPARE_BOOL_MEMBER_SWO( logicOpEnabled_        );
    LLGL_COMPARE_MEMBER_SWO     ( logicOp_               );
    #endif
    LLGL_COMPARE_MEMBER_SWO     ( numDrawBuffers_        );

    for (decltype(numDrawBuffers_) i = 0; i < numDrawBuffers_; ++i)
    {
        auto order = GLDrawBufferState::CompareSWO(lhs.drawBuffers_[i], rhs.drawBuffers_[i]);
        if (order != 0)
            return order;
    }

    return 0;
}


/*
 * ======= Private: =======
 */

void GLBlendState::BindDrawBufferStates(GLStateManager& stateMngr)
{
    if (numDrawBuffers_ == 1)
    {
        /* Bind blend states for all draw buffers */
        BindDrawBufferState(drawBuffers_[0]);
    }
    else if (numDrawBuffers_ > 1)
    {
        #ifdef GL_ARB_draw_buffers_blend
        if (HasExtension(GLExt::ARB_draw_buffers_blend))
        {
            /* Bind blend states for respective draw buffers directly via extension */
            for (GLuint i = 0; i < numDrawBuffers_; ++i)
                BindIndexedDrawBufferState(drawBuffers_[i], i);
        }
        else
        #endif // /GL_ARB_draw_buffers_blend
        {
            /* Bind blend states with emulated draw buffer setting */
            for (GLuint i = 0; i < numDrawBuffers_; ++i)
            {
                GLProfile::DrawBuffer(GLTypes::ToColorAttachment(i));
                BindDrawBufferState(drawBuffers_[i]);
            }

            /* Restore draw buffer settings for current render target */
            if (auto boundRenderTarget = stateMngr.GetBoundRenderTarget())
                boundRenderTarget->SetDrawBuffers();
        }
    }
}

void GLBlendState::BindDrawBufferColorMasks(GLStateManager& stateMngr)
{
    if (numDrawBuffers_ == 1)
    {
        /* Bind color mask for all draw buffers */
        BindDrawBufferColorMask(drawBuffers_[0]);
    }
    else if (numDrawBuffers_ > 1)
    {
        #ifdef GL_EXT_draw_buffers2
        if (HasExtension(GLExt::EXT_draw_buffers2))
        {
            /* Bind color mask for respective draw buffers directly via extension */
            for (GLuint i = 0; i < numDrawBuffers_; ++i)
                BindIndexedDrawBufferColorMask(drawBuffers_[i], i);
        }
        else
        #endif // /GL_EXT_draw_buffers2
        {
            /* Bind color masks with emulated draw buffer setting */
            for (GLuint i = 0; i < numDrawBuffers_; ++i)
            {
                GLProfile::DrawBuffer(GLTypes::ToColorAttachment(i));
                BindDrawBufferColorMask(drawBuffers_[i]);
            }

            /* Restore draw buffer settings for current render target */
            if (auto boundRenderTarget = stateMngr.GetBoundRenderTarget())
                boundRenderTarget->SetDrawBuffers();
        }
    }
}

//TODO: GL_BLEND should be enabled/disabled with state manager
void GLBlendState::BindDrawBufferState(const GLDrawBufferState& state)
{
    glColorMask(state.colorMask[0], state.colorMask[1], state.colorMask[2], state.colorMask[3]);
    if (state.blendEnabled)
    {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(state.srcColor, state.dstColor, state.srcAlpha, state.dstAlpha);
        glBlendEquationSeparate(state.funcColor, state.funcAlpha);
    }
    else
        glDisable(GL_BLEND);
}

//TODO: GL_BLEND should be enabled/disabled with state manager
void GLBlendState::BindIndexedDrawBufferState(const GLDrawBufferState& state, GLuint index)
{
    #ifdef LLGL_GLEXT_DRAW_BUFFERS_BLEND

    glColorMaski(index, state.colorMask[0], state.colorMask[1], state.colorMask[2], state.colorMask[3]);
    if (state.blendEnabled)
    {
        glEnablei(GL_BLEND, index);
        glBlendFuncSeparatei(index, state.srcColor, state.dstColor, state.srcAlpha, state.dstAlpha);
        glBlendEquationSeparatei(index, state.funcColor, state.funcAlpha);
    }
    else
        glDisablei(GL_BLEND, index);

    #endif // /LLGL_GLEXT_DRAW_BUFFERS_BLEND
}

void GLBlendState::BindDrawBufferColorMask(const GLDrawBufferState& state)
{
    glColorMask(state.colorMask[0], state.colorMask[1], state.colorMask[2], state.colorMask[3]);
}

void GLBlendState::BindIndexedDrawBufferColorMask(const GLDrawBufferState& state, GLuint index)
{
    #ifdef LLGL_GLEXT_DRAW_BUFFERS2

    glColorMaski(index, state.colorMask[0], state.colorMask[1], state.colorMask[2], state.colorMask[3]);

    #endif // /LLGL_GLEXT_DRAW_BUFFERS2
}


/*
 * GLDrawBufferState struct
 */

void GLBlendState::GLDrawBufferState::Convert(GLDrawBufferState& dst, const BlendTargetDescriptor& src)
{
    dst.blendEnabled    = GLBoolean(src.blendEnabled);
    dst.srcColor        = GLTypes::Map(src.srcColor);
    dst.dstColor        = GLTypes::Map(src.dstColor);
    dst.funcColor       = GLTypes::Map(src.colorArithmetic);
    dst.srcAlpha        = GLTypes::Map(src.srcAlpha);
    dst.dstAlpha        = GLTypes::Map(src.dstAlpha);
    dst.funcAlpha       = GLTypes::Map(src.alphaArithmetic);
    dst.colorMask[0]    = GLBoolean(src.colorMask.r);
    dst.colorMask[1]    = GLBoolean(src.colorMask.g);
    dst.colorMask[2]    = GLBoolean(src.colorMask.b);
    dst.colorMask[3]    = GLBoolean(src.colorMask.a);
}

int GLBlendState::GLDrawBufferState::CompareSWO(const GLDrawBufferState& lhs, const GLDrawBufferState& rhs)
{
    LLGL_COMPARE_BOOL_MEMBER_SWO( blendEnabled );
    LLGL_COMPARE_MEMBER_SWO     ( srcColor     );
    LLGL_COMPARE_MEMBER_SWO     ( dstColor     );
    LLGL_COMPARE_MEMBER_SWO     ( funcColor    );
    LLGL_COMPARE_MEMBER_SWO     ( srcAlpha     );
    LLGL_COMPARE_MEMBER_SWO     ( dstAlpha     );
    LLGL_COMPARE_MEMBER_SWO     ( funcAlpha    );
    LLGL_COMPARE_MEMBER_SWO     ( colorMask[0] );
    LLGL_COMPARE_MEMBER_SWO     ( colorMask[1] );
    LLGL_COMPARE_MEMBER_SWO     ( colorMask[2] );
    LLGL_COMPARE_MEMBER_SWO     ( colorMask[3] );
    return 0;
}


} // /namespace LLGL



// ================================================================================
