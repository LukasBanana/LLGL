/*
 * GLDepthStencilState.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLDepthStencilState.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../GLCore.h"
#include "../GLTypes.h"
#include "../../../Core/HelperMacros.h"
#include "GLStateManager.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


GLDepthStencilState::GLDepthStencilState(const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc)
{
    /* Convert depth states */
    depthTestEnabled_   = depthDesc.testEnabled;
    depthMask_          = GLBoolean(depthDesc.writeEnabled);
    depthFunc_          = GLTypes::Map(depthDesc.compareOp);

    /* Convert stencil states */
    stencilTestEnabled_ = stencilDesc.testEnabled;
    referenceDynamic_   = stencilDesc.referenceDynamic;

    GLStencilFaceState::Convert(stencilFront_, stencilDesc.front, stencilDesc.referenceDynamic);
    GLStencilFaceState::Convert(stencilBack_, stencilDesc.back, stencilDesc.referenceDynamic);

    independentStencilFaces_ = (GLStencilFaceState::CompareSWO(stencilFront_, stencilBack_) != 0);
}

void GLDepthStencilState::Bind(GLStateManager& stateMngr)
{
    /* Setup depth state */
    if (depthTestEnabled_)
    {
        stateMngr.Enable(GLState::DEPTH_TEST);
        stateMngr.SetDepthFunc(depthFunc_);
    }
    else
        stateMngr.Disable(GLState::DEPTH_TEST);

    stateMngr.SetDepthMask(depthMask_);

    /* Setup stencil state */
    if (stencilTestEnabled_)
    {
        stateMngr.Enable(GLState::STENCIL_TEST);
        if (independentStencilFaces_)
        {
            BindStencilFaceState(stencilFront_, GL_FRONT);
            BindStencilFaceState(stencilBack_, GL_BACK);
        }
        else
            BindStencilState(stencilFront_);
    }
    else
        stateMngr.Disable(GLState::STENCIL_TEST);
}

void GLDepthStencilState::BindStencilRefOnly(GLint ref, GLenum face)
{
    if (independentStencilFaces_)
    {
        switch (face)
        {
            case GL_FRONT_AND_BACK:
                glStencilFuncSeparate(GL_FRONT, stencilFront_.func, ref, stencilFront_.mask);
                glStencilFuncSeparate(GL_BACK, stencilBack_.func, ref, stencilBack_.mask);
                break;
            case GL_FRONT:
                glStencilFuncSeparate(GL_FRONT, stencilFront_.func, ref, stencilFront_.mask);
                break;
            case GL_BACK:
                glStencilFuncSeparate(GL_BACK, stencilBack_.func, ref, stencilBack_.mask);
                break;
        }
    }
    else
    {
        switch (face)
        {
            case GL_FRONT_AND_BACK:
                glStencilFunc(stencilFront_.func, ref, stencilFront_.mask);
                break;
            case GL_FRONT:
                glStencilFuncSeparate(GL_FRONT, stencilFront_.func, ref, stencilFront_.mask);
                break;
            case GL_BACK:
                glStencilFuncSeparate(GL_BACK, stencilBack_.func, ref, stencilBack_.mask);
                break;
        }
    }
}

int GLDepthStencilState::CompareSWO(const GLDepthStencilState& rhs) const
{
    const auto& lhs = *this;

    LLGL_COMPARE_BOOL_MEMBER_SWO( depthTestEnabled_ );
    if (depthTestEnabled_)
    {
        LLGL_COMPARE_MEMBER_SWO( depthMask_ );
        LLGL_COMPARE_MEMBER_SWO( depthFunc_ );
    }

    LLGL_COMPARE_BOOL_MEMBER_SWO( stencilTestEnabled_ );
    if (stencilTestEnabled_)
    {
        LLGL_COMPARE_BOOL_MEMBER_SWO( independentStencilFaces_ );

        {
            auto order = GLStencilFaceState::CompareSWO(stencilFront_, rhs.stencilFront_);
            if (order != 0)
                return order;
        }

        if (!independentStencilFaces_)
        {
            auto order = GLStencilFaceState::CompareSWO(stencilBack_, rhs.stencilBack_);
            if (order != 0)
                return order;
        }
    }

    return 0;
}


/*
 * ======= Private: =======
 */

void GLDepthStencilState::BindStencilFaceState(const GLStencilFaceState& state, GLenum face)
{
    glStencilOpSeparate(face, state.sfail, state.dpfail, state.dppass);
    if (!referenceDynamic_)
        glStencilFuncSeparate(face, state.func, state.ref, state.mask);
    glStencilMaskSeparate(face, state.writeMask);
}

void GLDepthStencilState::BindStencilState(const GLStencilFaceState& state)
{
    glStencilOp(state.sfail, state.dpfail, state.dppass);
    if (!referenceDynamic_)
        glStencilFunc(state.func, state.ref, state.mask);
    glStencilMask(state.writeMask);
}


/*
 * GLDrawBufferState struct
 */

void GLDepthStencilState::GLStencilFaceState::Convert(GLStencilFaceState& dst, const StencilFaceDescriptor& src, bool referenceDynamic)
{
    dst.sfail        = GLTypes::Map(src.stencilFailOp);
    dst.dpfail       = GLTypes::Map(src.depthFailOp);
    dst.dppass       = GLTypes::Map(src.depthPassOp);
    dst.func         = GLTypes::Map(src.compareOp);
    dst.ref          = (referenceDynamic ? 0 : static_cast<GLint>(src.reference));
    dst.mask         = src.readMask;
    dst.writeMask    = src.writeMask;
}

int GLDepthStencilState::GLStencilFaceState::CompareSWO(const GLStencilFaceState& lhs, const GLStencilFaceState& rhs)
{
    LLGL_COMPARE_MEMBER_SWO( sfail     );
    LLGL_COMPARE_MEMBER_SWO( dpfail    );
    LLGL_COMPARE_MEMBER_SWO( dppass    );
    LLGL_COMPARE_MEMBER_SWO( func      );
    LLGL_COMPARE_MEMBER_SWO( ref       );
    LLGL_COMPARE_MEMBER_SWO( mask      );
    LLGL_COMPARE_MEMBER_SWO( writeMask );
    return 0;
}


} // /namespace LLGL



// ================================================================================
