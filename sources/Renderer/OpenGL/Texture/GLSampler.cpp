/*
 * GLSampler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSampler.h"
#include "../GLTypes.h"
#include "../GLObjectUtils.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


GLSampler::GLSampler()
{
    glGenSamplers(1, &id_);
}

GLSampler::~GLSampler()
{
    glDeleteSamplers(1, &id_);
    GLStateManager::Get().NotifySamplerRelease(id_);
}

void GLSampler::SetName(const char* name)
{
    GLSetObjectLabel(GL_SAMPLER, GetID(), name);
}

static GLenum GetGLSamplerMinFilter(const SamplerDescriptor& desc)
{
    if (desc.mipMapping)
        return GLTypes::Map(desc.minFilter, desc.mipMapFilter);
    else
        return GLTypes::Map(desc.minFilter);
}

void GLSampler::SetDesc(const SamplerDescriptor& desc)
{
    /* Set texture coordinate wrap modes */
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_S, GLTypes::Map(desc.addressModeU));
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_T, GLTypes::Map(desc.addressModeV));
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_R, GLTypes::Map(desc.addressModeW));

    /* Set filter states */
    glSamplerParameteri(id_, GL_TEXTURE_MIN_FILTER, GetGLSamplerMinFilter(desc));
    glSamplerParameteri(id_, GL_TEXTURE_MAG_FILTER, GLTypes::Map(desc.magFilter));
    #ifdef LLGL_OPENGL
    glSamplerParameterf(id_, GL_TEXTURE_MAX_ANISOTROPY_EXT, static_cast<float>(desc.maxAnisotropy));
    #endif

    /* Set MIP-map level selection */
    glSamplerParameterf(id_, GL_TEXTURE_MIN_LOD, desc.minLOD);
    glSamplerParameterf(id_, GL_TEXTURE_MAX_LOD, desc.maxLOD);
    #ifdef LLGL_OPENGL
    glSamplerParameterf(id_, GL_TEXTURE_LOD_BIAS, desc.mipMapLODBias);
    #endif

    /* Set compare operation */
    if (desc.compareEnabled)
    {
        glSamplerParameteri(id_, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glSamplerParameteri(id_, GL_TEXTURE_COMPARE_FUNC, GLTypes::Map(desc.compareOp));
    }
    else
        glSamplerParameteri(id_, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    /* Set border color */
    #if defined LLGL_OPENGL || defined GL_ES_VERSION_3_2
    glSamplerParameterfv(id_, GL_TEXTURE_BORDER_COLOR, desc.borderColor.Ptr());
    #endif
}


} // /namespace LLGL



// ================================================================================
