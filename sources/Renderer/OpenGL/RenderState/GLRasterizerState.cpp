/*
 * GLRasterizerState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLRasterizerState.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../GLCore.h"
#include "../GLTypes.h"
#include "../../../Core/MacroUtils.h"
#include "../../../Core/Exception.h"
#include "GLStateManager.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


static GLState ToPolygonOffsetState(const PolygonMode mode)
{
    switch (mode)
    {
        case PolygonMode::Fill:         return GLState::PolygonOffsetFill;
        #ifdef LLGL_OPENGL
        case PolygonMode::Wireframe:    return GLState::PolygonOffsetLine;
        case PolygonMode::Points:       return GLState::PolygonOffsetPoint;
        #else
        case PolygonMode::Wireframe:    break;
        case PolygonMode::Points:       break;
        #endif
    }
    LLGL_TRAP("failed to map LLGL::PolygonMode(%d) to polygon offset mode (GL_POLYGON_OFFSET_FILL, GL_POLYGON_OFFSET_LINE, or GL_POLYGON_OFFSET_POINT)", static_cast<int>(mode));
}

static bool IsPolygonOffsetEnabled(const DepthBiasDescriptor& desc)
{
    /* Ignore clamp factor for this check, since it's useless without the other two parameters */
    return (desc.slopeFactor != 0.0f || desc.constantFactor != 0.0f);
}

GLRasterizerState::GLRasterizerState(const RasterizerDescriptor& desc)
{
    #ifdef LLGL_OPENGL
    polygonMode_            = GLTypes::Map(desc.polygonMode);
    depthClampEnabled_      = desc.depthClampEnabled;
    #endif

    cullFace_               = GLTypes::Map(desc.cullMode);
    frontFace_              = (desc.frontCCW ? GL_CCW : GL_CW);
    rasterizerDiscard_      = desc.discardEnabled;
    scissorTestEnabled_     = desc.scissorTestEnabled;
    multiSampleEnabled_     = desc.multiSampleEnabled;
    lineSmoothEnabled_      = desc.antiAliasedLineEnabled;
    lineWidth_              = desc.lineWidth;
    polygonOffsetEnabled_   = IsPolygonOffsetEnabled(desc.depthBias);
    polygonOffsetMode_      = ToPolygonOffsetState(desc.polygonMode);
    polygonOffsetFactor_    = desc.depthBias.slopeFactor;
    polygonOffsetUnits_     = desc.depthBias.constantFactor;
    polygonOffsetClamp_     = desc.depthBias.clamp;

    #ifdef LLGL_GL_ENABLE_VENDOR_EXT
    conservativeRaster_     = desc.conservativeRasterization;
    #endif
}

void GLRasterizerState::Bind(GLStateManager& stateMngr)
{
    #ifdef LLGL_OPENGL
    stateMngr.SetPolygonMode(polygonMode_);
    stateMngr.Set(GLState::DepthClamp, depthClampEnabled_);
    stateMngr.Set(GLState::Multisample, multiSampleEnabled_);
    stateMngr.Set(GLState::LineSmooth, lineSmoothEnabled_);
    #endif

    stateMngr.SetFrontFace(frontFace_);
    stateMngr.SetLineWidth(lineWidth_);

    stateMngr.Set(GLState::RasterizerDiscard, rasterizerDiscard_);
    stateMngr.Set(GLState::ScissorTest, scissorTestEnabled_);

    if (cullFace_ != 0)
    {
        stateMngr.Enable(GLState::CullFace);
        stateMngr.SetCullFace(cullFace_);
    }
    else
        stateMngr.Disable(GLState::CullFace);

    if (polygonOffsetEnabled_)
    {
        stateMngr.Enable(polygonOffsetMode_);
        stateMngr.SetPolygonOffset(polygonOffsetFactor_, polygonOffsetUnits_, polygonOffsetClamp_);
    }
    else
        stateMngr.Disable(polygonOffsetMode_);

    #ifdef LLGL_GL_ENABLE_VENDOR_EXT
    stateMngr.Set(GLStateExt::ConservativeRasterization, conservativeRaster_);
    #endif
}

void GLRasterizerState::BindFrontFaceOnly(GLStateManager& stateMngr)
{
    stateMngr.SetFrontFace(frontFace_);
}

int GLRasterizerState::CompareSWO(const GLRasterizerState& lhs, const GLRasterizerState& rhs)
{
    #ifdef LLGL_OPENGL
    LLGL_COMPARE_MEMBER_SWO     ( polygonMode_          );
    LLGL_COMPARE_BOOL_MEMBER_SWO( depthClampEnabled_    );
    #endif

    LLGL_COMPARE_MEMBER_SWO     ( cullFace_             );
    LLGL_COMPARE_MEMBER_SWO     ( frontFace_            );
    LLGL_COMPARE_BOOL_MEMBER_SWO( scissorTestEnabled_   );
    LLGL_COMPARE_BOOL_MEMBER_SWO( multiSampleEnabled_   );
    LLGL_COMPARE_BOOL_MEMBER_SWO( lineSmoothEnabled_    );
    LLGL_COMPARE_MEMBER_SWO     ( lineWidth_            );
    LLGL_COMPARE_BOOL_MEMBER_SWO( polygonOffsetEnabled_ );
    LLGL_COMPARE_MEMBER_SWO     ( polygonOffsetMode_    );
    LLGL_COMPARE_MEMBER_SWO     ( polygonOffsetFactor_  );
    LLGL_COMPARE_MEMBER_SWO     ( polygonOffsetUnits_   );
    LLGL_COMPARE_MEMBER_SWO     ( polygonOffsetClamp_   );

    #ifdef LLGL_GL_ENABLE_VENDOR_EXT
    LLGL_COMPARE_BOOL_MEMBER_SWO( conservativeRaster_   );
    #endif

    return 0;
}


} // /namespace LLGL



// ================================================================================
