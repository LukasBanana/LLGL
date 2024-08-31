/*
 * GLDepthStencilState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLDepthStencilState.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../GLCore.h"
#include "../GLTypes.h"
#include "../../../Core/MacroUtils.h"
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

    #if LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES
    independentStencilFaces_ = (GLStencilFaceState::CompareSWO(stencilFront_, stencilBack_) != 0);
    #endif
}

void GLDepthStencilState::Bind(GLStateManager& stateMngr)
{
    /* Setup depth state */
    if (depthTestEnabled_)
    {
        stateMngr.Enable(GLState::DepthTest);
        stateMngr.SetDepthFunc(depthFunc_);
    }
    else
        stateMngr.Disable(GLState::DepthTest);

    stateMngr.SetDepthMask(depthMask_);

    /* Setup stencil state */
    if (stencilTestEnabled_)
    {
        stateMngr.Enable(GLState::StencilTest);
        #if LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES
        if (independentStencilFaces_)
        {
            BindStencilFaceState(stencilFront_, GL_FRONT);
            BindStencilFaceState(stencilBack_, GL_BACK);
        }
        else
        #endif // /LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES
        {
            BindStencilState(stencilFront_);
        }
    }
    else
        stateMngr.Disable(GLState::StencilTest);
}

void GLDepthStencilState::BindStencilRefOnly(GLint ref, GLenum face)
{
    #if LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES
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
    #endif // /LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES
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

void GLDepthStencilState::BindStencilWriteMaskOnly()
{
    if (stencilTestEnabled_)
    {
        #if LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES
        if (independentStencilFaces_)
        {
            glStencilMaskSeparate(GL_FRONT, stencilFront_.writeMask);
            glStencilMaskSeparate(GL_BACK, stencilBack_.writeMask);
        }
        else
        #endif // /LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES
        {
            glStencilMask(stencilFront_.writeMask);
        }
    }
}

int GLDepthStencilState::CompareSWO(const GLDepthStencilState& lhs, const GLDepthStencilState& rhs)
{
    LLGL_COMPARE_BOOL_MEMBER_SWO( depthTestEnabled_ );
    if (lhs.depthTestEnabled_)
    {
        LLGL_COMPARE_MEMBER_SWO( depthMask_ );
        LLGL_COMPARE_MEMBER_SWO( depthFunc_ );
    }

    LLGL_COMPARE_BOOL_MEMBER_SWO( stencilTestEnabled_ );
    if (lhs.stencilTestEnabled_)
    {
        #if LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES
        LLGL_COMPARE_BOOL_MEMBER_SWO( independentStencilFaces_ );
        #endif // /LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES

        {
            int order = GLStencilFaceState::CompareSWO(lhs.stencilFront_, rhs.stencilFront_);
            if (order != 0)
                return order;
        }

        #if LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES
        if (!lhs.independentStencilFaces_)
        {
            int order = GLStencilFaceState::CompareSWO(lhs.stencilBack_, rhs.stencilBack_);
            if (order != 0)
                return order;
        }
        #endif // /LLGL_SUPPORTS_INDEPENDENT_STENCIL_FACES
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
