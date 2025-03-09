/*
 * GLEmulatedSampler.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLEmulatedSampler.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include "../../../Core/MacroUtils.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Backend/OpenGL/NativeHandle.h>
#include <algorithm>


namespace LLGL
{


bool GLEmulatedSampler::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (auto* nativeHandleGL = GetTypedNativeHandle<OpenGL::ResourceNativeHandle>(nativeHandle, nativeHandleSize))
    {
        nativeHandleGL->type    = OpenGL::ResourceNativeType::EmulatedSampler;
        nativeHandleGL->id      = 0;
        return true;
    }
    return false;
}

#if LLGL_SAMPLER_BORDER_COLOR

static bool IsGLTextureWrapUsingBorder(GLenum mode)
{
    /* Accroding to GL2.x spec: "Border texture elements are accessed only if wrapping is set to GL_CLAMP or GL_CLAMP_TO_BORDER" */
    return (mode == GL_CLAMP || mode == GL_CLAMP_TO_BORDER);
}

#endif // /LLGL_SAMPLER_BORDER_COLOR

void GLEmulatedSampler::SamplerParameters(const SamplerDescriptor& desc)
{
    /* Store texture coordinate wrap modes */
    wrapS_              = GLTypes::Map(desc.addressModeU);
    wrapT_              = GLTypes::Map(desc.addressModeV);
    wrapR_              = GLTypes::Map(desc.addressModeW);

    /* Store filter states */
    minFilter_          = GLTypes::ToSamplerMinFilter(desc);
    magFilter_          = GLTypes::Map(desc.magFilter);
    #if LLGL_OPENGL
    maxAnisotropy_      = static_cast<float>(desc.maxAnisotropy);
    #endif

    /* Store MIP-map level selection */
    minLod_             = desc.minLOD;
    maxLod_             = desc.maxLOD;
    #if LLGL_OPENGL
    lodBias_            = desc.mipMapLODBias;
    #endif

    /* Store compare operation */
    if (desc.compareEnabled)
    {
        #if LLGL_GL_ENABLE_OPENGL2X
        compareMode_    = GL_COMPARE_R_TO_TEXTURE;
        #else
        compareMode_    = GL_COMPARE_REF_TO_TEXTURE;
        #endif
        compareFunc_    = GLTypes::Map(desc.compareOp);
    }
    else
        compareMode_    = GL_NONE;

    #if LLGL_SAMPLER_BORDER_COLOR
    /* Set border color */
    borderColor_[0]     = (std::max)(0.0f, (std::min)(desc.borderColor[0], 1.0f));
    borderColor_[1]     = (std::max)(0.0f, (std::min)(desc.borderColor[1], 1.0f));
    borderColor_[2]     = (std::max)(0.0f, (std::min)(desc.borderColor[2], 1.0f));
    borderColor_[3]     = (std::max)(0.0f, (std::min)(desc.borderColor[3], 1.0f));
    borderColorUsed_    = (IsGLTextureWrapUsingBorder(wrapS_) || IsGLTextureWrapUsingBorder(wrapT_) || IsGLTextureWrapUsingBorder(wrapR_));
    #endif // /LLGL_SAMPLER_BORDER_COLOR
}

static void GLSetTexParameteri(GLenum target, GLenum param, GLint value)
{
    glTexParameteri(target, param, value);
}

static void GLSetTexParameterf(GLenum target, GLenum param, GLfloat value)
{
    glTexParameterf(target, param, value);
}

static void GLSetTexParameterfv(GLenum target, GLenum param, const GLfloat* values)
{
    glTexParameterfv(target, param, values);
}

static void GLChangeTexParameteri(GLenum target, GLenum param, GLint value, GLint prevValue)
{
    if (value != prevValue)
        glTexParameteri(target, param, value);
}

static void GLChangeTexParameterf(GLenum target, GLenum param, GLfloat value, GLfloat prevValue)
{
    if (value != prevValue)
        glTexParameterf(target, param, value);
}

static void GLChangeTexParameterfv(GLenum target, GLenum param, const GLfloat (&values)[4], const GLfloat (&prevValues)[4])
{
    if (values[0] != prevValues[0] ||
        values[1] != prevValues[1] ||
        values[2] != prevValues[2] ||
        values[3] != prevValues[3])
    {
        glTexParameterfv(target, param, values);
    }
}

void GLEmulatedSampler::BindTexParameters(GLenum target, const GLEmulatedSampler* prevSampler) const
{
    if (prevSampler != nullptr)
    {
        /* Set parameters that have changed from previous sampler */
        GLChangeTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS_, prevSampler->wrapS_);
        GLChangeTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT_, prevSampler->wrapT_);
        GLChangeTexParameteri(target, GL_TEXTURE_WRAP_R, wrapR_, prevSampler->wrapR_);
        GLChangeTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter_, prevSampler->minFilter_);
        GLChangeTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter_, prevSampler->magFilter_);
        #if LLGL_OPENGL
        GLChangeTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy_, prevSampler->maxAnisotropy_);
        #endif
        GLChangeTexParameterf(target, GL_TEXTURE_MIN_LOD, minLod_, prevSampler->minLod_);
        GLChangeTexParameterf(target, GL_TEXTURE_MAX_LOD, maxLod_, prevSampler->maxLod_);
        #if LLGL_OPENGL
        GLChangeTexParameterf(target, GL_TEXTURE_LOD_BIAS, lodBias_, prevSampler->lodBias_);
        #endif
        GLChangeTexParameteri(target, GL_TEXTURE_COMPARE_MODE, compareMode_, prevSampler->compareMode_);
        if (compareMode_ != GL_NONE)
            GLChangeTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, compareFunc_, prevSampler->compareFunc_);
        #if LLGL_SAMPLER_BORDER_COLOR
        if (borderColorUsed_)
            GLChangeTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor_, prevSampler->borderColor_);
        #endif
    }
    else
    {
        /* Initialize all parameters */
        GLSetTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS_);
        GLSetTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT_);
        GLSetTexParameteri(target, GL_TEXTURE_WRAP_R, wrapR_);
        GLSetTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter_);
        GLSetTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter_);
        #if LLGL_OPENGL
        GLSetTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy_);
        #endif
        GLSetTexParameterf(target, GL_TEXTURE_MIN_LOD, minLod_);
        GLSetTexParameterf(target, GL_TEXTURE_MAX_LOD, maxLod_);
        #if LLGL_OPENGL
        GLSetTexParameterf(target, GL_TEXTURE_LOD_BIAS, lodBias_);
        #endif
        GLSetTexParameteri(target, GL_TEXTURE_COMPARE_MODE, compareMode_);
        GLSetTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, compareFunc_);
        #if LLGL_SAMPLER_BORDER_COLOR
        GLSetTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor_);
        #endif
    }
}

int GLEmulatedSampler::CompareSWO(const GLEmulatedSampler& lhs, const GLEmulatedSampler& rhs)
{
    LLGL_COMPARE_MEMBER_SWO( wrapS_         );
    LLGL_COMPARE_MEMBER_SWO( wrapT_         );
    LLGL_COMPARE_MEMBER_SWO( wrapR_         );
    LLGL_COMPARE_MEMBER_SWO( minFilter_     );
    LLGL_COMPARE_MEMBER_SWO( magFilter_     );
    #if LLGL_OPENGL
    LLGL_COMPARE_MEMBER_SWO( maxAnisotropy_ );
    #endif
    LLGL_COMPARE_MEMBER_SWO( minLod_        );
    LLGL_COMPARE_MEMBER_SWO( maxLod_        );
    #if LLGL_OPENGL
    LLGL_COMPARE_MEMBER_SWO( lodBias_       );
    #endif
    LLGL_COMPARE_MEMBER_SWO( compareMode_   );
    if (lhs.compareMode_ != GL_NONE)
    {
        /* Only compare comparison-function if compare-mode is enabled */
        LLGL_COMPARE_MEMBER_SWO( compareFunc_ );
    }
    #if LLGL_SAMPLER_BORDER_COLOR
    if (lhs.borderColorUsed_)
    {
        LLGL_COMPARE_MEMBER_SWO( borderColor_[0] );
        LLGL_COMPARE_MEMBER_SWO( borderColor_[1] );
        LLGL_COMPARE_MEMBER_SWO( borderColor_[2] );
        LLGL_COMPARE_MEMBER_SWO( borderColor_[3] );
    }
    #endif // /LLGL_SAMPLER_BORDER_COLOR
    return 0;
}


} // /namespace LLGL



// ================================================================================
