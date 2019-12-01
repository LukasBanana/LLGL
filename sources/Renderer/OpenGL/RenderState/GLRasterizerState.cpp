/*
 * GLRasterizerState.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRasterizerState.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../GLCore.h"
#include "../GLTypes.h"
#include "../../../Core/HelperMacros.h"
#include "GLStateManager.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


static GLState ToPolygonOffsetState(const PolygonMode mode)
{
    switch (mode)
    {
        case PolygonMode::Fill:         return GLState::POLYGON_OFFSET_FILL;
        #ifdef LLGL_OPENGL
        case PolygonMode::Wireframe:    return GLState::POLYGON_OFFSET_LINE;
        case PolygonMode::Points:       return GLState::POLYGON_OFFSET_POINT;
        #else
        case PolygonMode::Wireframe:    break;
        case PolygonMode::Points:       break;
        #endif
    }
    throw std::invalid_argument("failed to map 'PolygonMode' to polygon offset mode (GL_POLYGON_OFFSET_FILL, GL_POLYGON_OFFSET_LINE, or GL_POLYGON_OFFSET_POINT)");
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
    stateMngr.Set(GLState::DEPTH_CLAMP, depthClampEnabled_);
    stateMngr.Set(GLState::MULTISAMPLE, multiSampleEnabled_);
    stateMngr.Set(GLState::LINE_SMOOTH, lineSmoothEnabled_);
    #endif
    
    stateMngr.SetFrontFace(frontFace_);
    stateMngr.SetLineWidth(lineWidth_);

    stateMngr.Set(GLState::RASTERIZER_DISCARD, rasterizerDiscard_);
    stateMngr.Set(GLState::SCISSOR_TEST, scissorTestEnabled_);

    if (cullFace_ != 0)
    {
        stateMngr.Enable(GLState::CULL_FACE);
        stateMngr.SetCullFace(cullFace_);
    }
    else
        stateMngr.Disable(GLState::CULL_FACE);

    if (polygonOffsetEnabled_)
    {
        stateMngr.Enable(polygonOffsetMode_);
        stateMngr.SetPolygonOffset(polygonOffsetFactor_, polygonOffsetUnits_, polygonOffsetClamp_);
    }
    else
        stateMngr.Disable(polygonOffsetMode_);

    #ifdef LLGL_GL_ENABLE_VENDOR_EXT
    stateMngr.Set(GLStateExt::CONSERVATIVE_RASTERIZATION, conservativeRaster_);
    #endif
}

int GLRasterizerState::CompareSWO(const GLRasterizerState& rhs) const
{
    const auto& lhs = *this;

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
