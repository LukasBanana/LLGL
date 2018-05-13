/*
 * GLSampler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSampler.h"
#include "../Ext/GLExtensions.h"
#include "../../GLCommon/GLTypes.h"


namespace LLGL
{


GLSampler::GLSampler()
{
    glGenSamplers(1, &id_);
}

GLSampler::~GLSampler()
{
    glDeleteSamplers(1, &id_);
}

using namespace GLTypes;

void GLSampler::SetDesc(const SamplerDescriptor& desc)
{
    /* Set texture coordinate wrap modes */
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_S, Map(desc.textureWrapU));
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_T, Map(desc.textureWrapV));
    glSamplerParameteri(id_, GL_TEXTURE_WRAP_R, Map(desc.textureWrapW));

    /* Set filter states */
    glSamplerParameteri(id_, GL_TEXTURE_MIN_FILTER, (desc.mipMapping ? Map(desc.minFilter, desc.mipMapFilter) : Map(desc.minFilter)));
    glSamplerParameteri(id_, GL_TEXTURE_MAG_FILTER, Map(desc.magFilter));
    glSamplerParameterf(id_, GL_TEXTURE_MAX_ANISOTROPY_EXT, static_cast<float>(desc.maxAnisotropy));

    /* Set MIP-map level selection */
    glSamplerParameterf(id_, GL_TEXTURE_MIN_LOD, desc.minLOD);
    glSamplerParameterf(id_, GL_TEXTURE_MAX_LOD, desc.maxLOD);
    glSamplerParameterf(id_, GL_TEXTURE_LOD_BIAS, desc.mipMapLODBias);

    /* Set compare operation */
    if (desc.compareEnabled)
    {
        glSamplerParameteri(id_, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glSamplerParameteri(id_, GL_TEXTURE_COMPARE_FUNC, Map(desc.compareOp));
    }
    else
        glSamplerParameteri(id_, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    /* Set border color */
    glSamplerParameterfv(id_, GL_TEXTURE_BORDER_COLOR, desc.borderColor.Ptr());
}


} // /namespace LLGL



// ================================================================================
