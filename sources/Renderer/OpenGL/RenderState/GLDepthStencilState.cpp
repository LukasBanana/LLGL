/*
 * GLDepthStencilState.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLDepthStencilState.h"
#include "../Ext/GLExtensions.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../../GLCommon/GLCore.h"
#include "../../GLCommon/GLTypes.h"
#include "../../../Core/HelperMacros.h"
#include "GLStateManager.h"


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
    GLStencilFaceState::Convert(stencilFront_, stencilDesc.front);
    GLStencilFaceState::Convert(stencilBack_, stencilDesc.back);

    independentStencilFaces_ = (GLStencilFaceState::CompareSWO(stencilFront_, stencilBack_) == 0);
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
    glStencilFuncSeparate(face, state.func, state.ref, state.mask);
    glStencilMaskSeparate(face, state.writeMask);
}

void GLDepthStencilState::BindStencilState(const GLStencilFaceState& state)
{
    glStencilOp(state.sfail, state.dpfail, state.dppass);
    glStencilFunc(state.func, state.ref, state.mask);
    glStencilMask(state.writeMask);
}


/*
 * GLDrawBufferState struct
 */

void GLDepthStencilState::GLStencilFaceState::Convert(GLStencilFaceState& dst, const StencilFaceDescriptor& src)
{
    dst.sfail        = GLTypes::Map(src.stencilFailOp);
    dst.dpfail       = GLTypes::Map(src.depthFailOp);
    dst.dppass       = GLTypes::Map(src.depthPassOp);
    dst.func         = GLTypes::Map(src.compareOp);
    dst.ref          = static_cast<GLint>(src.reference);
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
