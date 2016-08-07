/*
 * GLRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include <LLGL/Desktop.h>
#include "GLStateManager.h"
#include "GLTypeConversion.h"
#include "GLExtensions.h"


namespace LLGL
{


GLRenderSystem::GLRenderSystem()
{
}

GLRenderSystem::~GLRenderSystem()
{
    Desktop::ResetVideoMode();
}

RenderContext* GLRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    /* Create new render context */
    return AddRenderContext(MakeUnique<GLRenderContext>(desc, window, nullptr), desc, window);
}

/* ----- Hardware buffers ------ */

VertexBuffer* GLRenderSystem::CreateVertexBuffer()
{
    return TakeOwnership(vertexBuffers_, MakeUnique<GLVertexBuffer>());
}

IndexBuffer* GLRenderSystem::CreateIndexBuffer()
{
    return TakeOwnership(indexBuffers_, MakeUnique<GLIndexBuffer>());
}

ConstantBuffer* GLRenderSystem::CreateConstantBuffer()
{
    return TakeOwnership(constantBuffers_, MakeUnique<GLConstantBuffer>());
}

void GLRenderSystem::WriteVertexBuffer(
    VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat)
{
    /* Bind vertex buffer */
    auto& vertexBufferGL = LLGL_CAST(GLVertexBuffer&, vertexBuffer);
    GLStateManager::active->BindBuffer(vertexBufferGL);

    /* Update buffer data and update new vertex format */
    vertexBufferGL.hwBuffer.BufferData(data, dataSize, GLTypeConversion::Map(usage));
    vertexBufferGL.UpdateVertexFormat(vertexFormat);
}

void GLRenderSystem::WriteIndexBuffer(
    IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat)
{
    /* Bind index buffer */
    auto& indexBufferGL = LLGL_CAST(GLIndexBuffer&, indexBuffer);
    GLStateManager::active->BindBuffer(indexBufferGL);

    /* Update buffer data and update new index format */
    indexBufferGL.hwBuffer.BufferData(data, dataSize, GLTypeConversion::Map(usage));
    indexBufferGL.UpdateIndexFormat(indexFormat);
}

void GLRenderSystem::WriteConstantBuffer(
    ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    /* Bind constant buffer */
    auto& constantBufferGL = LLGL_CAST(GLConstantBuffer&, constantBuffer);
    GLStateManager::active->BindBuffer(constantBufferGL);

    /* Update buffer data */
    constantBufferGL.hwBuffer.BufferData(data, dataSize, GLTypeConversion::Map(usage));
}

void GLRenderSystem::WriteVertexBufferSub(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    /* Bind and update vertex buffer */
    auto& vertexBufferGL = LLGL_CAST(GLVertexBuffer&, vertexBuffer);
    GLStateManager::active->BindBuffer(vertexBufferGL);
    vertexBufferGL.hwBuffer.BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

void GLRenderSystem::WriteIndexBufferSub(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    /* Bind index buffer */
    auto& indexBufferGL = LLGL_CAST(GLIndexBuffer&, indexBuffer);
    GLStateManager::active->BindBuffer(indexBufferGL);
    indexBufferGL.hwBuffer.BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

void GLRenderSystem::WriteConstantBufferSub(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    /* Bind constant buffer */
    auto& constantBufferGL = LLGL_CAST(GLConstantBuffer&, constantBuffer);
    GLStateManager::active->BindBuffer(constantBufferGL);
    constantBufferGL.hwBuffer.BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

/* ----- Textures ----- */

static void GLTexImage1D(GLenum internalFormat, int width, GLenum format, GLenum type, const void* data)
{
    glTexImage1D(GL_TEXTURE_1D, 0, internalFormat, width, 0, format, type, data);
}

static void GLTexImage2D(GLenum internalFormat, int width, int height, GLenum format, GLenum type, const void* data)
{
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
}

static void GLTexImage3D(GLenum internalFormat, int width, int height, int depth, GLenum format, GLenum type, const void* data)
{
    glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, width, height, depth, 0, format, type, data);
}

static void GLTexImage1DArray(GLenum internalFormat, int width, unsigned int layers, GLenum format, GLenum type, const void* data)
{
    glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, internalFormat, width, static_cast<GLsizei>(layers), 0, format, type, data);
}

static void GLTexImage2DArray(GLenum internalFormat, int width, int height, unsigned int layers, GLenum format, GLenum type, const void* data)
{
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat, width, height, static_cast<GLsizei>(layers), 0, format, type, data);
}

Texture* GLRenderSystem::CreateTexture()
{
    return TakeOwnership(textures_, MakeUnique<GLTexture>());
}

TextureDescriptor GLRenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    /* Bind texture */
    auto& textureGL = CheckedCast<const GLTexture&>(texture);
    GLStateManager::active->BindTexture(textureGL);

    /* Setup texture descriptor */
    TextureDescriptor desc;
    InitMemory(desc);

    desc.type = texture.GetType();

    /* Query texture size */
    auto target = GLTypeConversion::Map(texture.GetType());

    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &desc.texture3DDesc.width);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &desc.texture3DDesc.height);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH, &desc.texture3DDesc.depth);

    return desc;
}

void GLRenderSystem::WriteTexture1D(Texture& texture, const TextureFormat format, int width, const TextureDataDescriptor* textureData)
{
    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture1D);

    if (textureData)
    {
        /* Setup texture image from descriptor */
        GLTexImage1D(
            GLTypeConversion::Map(format),
            width,
            GLTypeConversion::Map(textureData->dataFormat),
            GLTypeConversion::Map(textureData->dataType),
            textureData->data
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(width);
        GLTexImage1D(GLTypeConversion::Map(format), width, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::WriteTexture2D(Texture& texture, const TextureFormat format, int width, int height, const TextureDataDescriptor* textureData)
{
    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture2D);

    if (textureData)
    {
        /* Setup texture image from descriptor */
        GLTexImage2D(
            GLTypeConversion::Map(format),
            width,
            height,
            GLTypeConversion::Map(textureData->dataFormat),
            GLTypeConversion::Map(textureData->dataType),
            textureData->data
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(width*height);
        GLTexImage2D(GLTypeConversion::Map(format), width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::WriteTexture3D(Texture& texture, const TextureFormat format, int width, int height, int depth, const TextureDataDescriptor* textureData)
{
    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture3D);

    if (textureData)
    {
        /* Setup texture image from descriptor */
        GLTexImage3D(
            GLTypeConversion::Map(format),
            width,
            height,
            depth,
            GLTypeConversion::Map(textureData->dataFormat),
            GLTypeConversion::Map(textureData->dataType),
            textureData->data
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(width*height*depth);
        GLTexImage3D(GLTypeConversion::Map(format), width, height, depth, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::WriteTextureCube(Texture& texture, const TextureFormat format, int width, int height, const TextureDataDescriptor* textureData)
{
    //todo...
}

void GLRenderSystem::WriteTexture1DArray(Texture& texture, const TextureFormat format, int width, unsigned int layers, const TextureDataDescriptor* textureData)
{
    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture1DArray);

    if (textureData)
    {
        /* Setup texture image from descriptor */
        GLTexImage1DArray(
            GLTypeConversion::Map(format),
            width,
            layers,
            GLTypeConversion::Map(textureData->dataFormat),
            GLTypeConversion::Map(textureData->dataType),
            textureData->data
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(width*static_cast<int>(layers));
        GLTexImage1DArray(GLTypeConversion::Map(format), width, layers, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::WriteTexture2DArray(Texture& texture, const TextureFormat format, int width, int height, unsigned int layers, const TextureDataDescriptor* textureData)
{
    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture2DArray);

    if (textureData)
    {
        /* Setup texture image from descriptor */
        GLTexImage2DArray(
            GLTypeConversion::Map(format),
            width,
            height,
            layers,
            GLTypeConversion::Map(textureData->dataFormat),
            GLTypeConversion::Map(textureData->dataType),
            textureData->data
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(width*height*static_cast<int>(layers));
        GLTexImage2DArray(GLTypeConversion::Map(format), width, height, layers, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::WriteTextureCubeArray(Texture& texture, const TextureFormat format, int width, int height, unsigned int layers, const TextureDataDescriptor* textureData)
{
    //todo...
}
        
void GLRenderSystem::WriteTexture1DSub(Texture& texture, const TextureFormat format, int width, const TextureDataDescriptor& textureData)
{
}

void GLRenderSystem::WriteTexture2DSub(Texture& texture, const TextureFormat format, int width, int height, const TextureDataDescriptor& textureData)
{
}

void GLRenderSystem::WriteTexture3DSub(Texture& texture, const TextureFormat format, int width, int height, int depth, const TextureDataDescriptor& textureData)
{
}

void GLRenderSystem::WriteTextureCubeSub(Texture& texture, const TextureFormat format, int width, int height, const TextureDataDescriptor& textureData)
{
}

void GLRenderSystem::WriteTexture1DArraySub(Texture& texture, const TextureFormat format, int width, unsigned int layers, const TextureDataDescriptor& textureData)
{
}

void GLRenderSystem::WriteTexture2DArraySub(Texture& texture, const TextureFormat format, int width, int height, unsigned int layers, const TextureDataDescriptor& textureData)
{
}

void GLRenderSystem::WriteTextureCubeArraySub(Texture& texture, const TextureFormat format, int width, int height, unsigned int layers, const TextureDataDescriptor& textureData)
{
}

void GLRenderSystem::ReadTexture(const Texture& texture, int mipLevel, ColorFormat dataFormat, DataType dataType, void* data)
{
    /* Bind texture */
    auto& textureGL = LLGL_CAST(const GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    /* Read image data from texture */
    glGetTexImage(
        GLTypeConversion::Map(textureGL.GetType()),
        mipLevel,
        GLTypeConversion::Map(dataFormat),
        GLTypeConversion::Map(dataType),
        data
    );
}

/* ----- Shader ----- */

Shader* GLRenderSystem::CreateShader(const ShaderType type)
{
    return TakeOwnership(shaders_, MakeUnique<GLShader>(type));
}

ShaderProgram* GLRenderSystem::CreateShaderProgram()
{
    return TakeOwnership(shaderPrograms_, MakeUnique<GLShaderProgram>());
}


/*
 * ======= Protected: =======
 */

RenderContext* GLRenderSystem::AddRenderContext(
    std::unique_ptr<GLRenderContext>&& renderContext, const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    /*
    If render context created it's own window then show it after creation,
    since anti-aliasing may force the window to be recreated several times
    */
    if (!window)
        renderContext->GetWindow().Show();

    /* Switch to fullscreen mode (if enabled) */
    if (desc.videoMode.fullscreen)
        Desktop::ChangeVideoMode(desc.videoMode);

    /* Load all OpenGL extensions for the first time */
    if (renderContexts_.empty())
        LoadGLExtensions(desc.profileOpenGL);

    /* Take ownership and return raw pointer */
    return TakeOwnership(renderContexts_, std::move(renderContext));
}


/*
 * ======= Private: =======
 */

bool GLRenderSystem::OnMakeCurrent(RenderContext* renderContext)
{
    if (renderContext)
    {
        auto renderContextGL = LLGL_CAST(GLRenderContext*, renderContext);
        return GLRenderContext::GLMakeCurrent(renderContextGL);
    }
    else
        return GLRenderContext::GLMakeCurrent(nullptr);
}

void GLRenderSystem::LoadGLExtensions(const ProfileOpenGLDescriptor& profileDesc)
{
    /* Load OpenGL extensions if not already done */
    if (extensionMap_.empty())
    {
        auto coreProfile = (profileDesc.extProfile && profileDesc.coreProfile);

        /* Query extensions and load all of them */
        extensionMap_ = QueryExtensions(coreProfile);
        LoadAllExtensions(extensionMap_);
    }
}

void GLRenderSystem::BindTextureAndSetType(GLTexture& textureGL, const TextureType type)
{
    if (textureGL.GetType() == TextureType::Undefined)
    {
        /* Set type of the specified texture for the first time */
        textureGL.SetType(type);
        GLStateManager::active->ForcedBindTexture(textureGL);

        /* Setup texture parameters for the first time */
        auto target = GLTypeConversion::Map(type);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else if (textureGL.GetType() != type)
    {
        /* Type change requires the texture to be recreated */
        textureGL.Recreate();
        GLStateManager::active->ForcedBindTexture(textureGL);
    }
    else
    {
        /* Type already set -> just bind texture */
        GLStateManager::active->BindTexture(textureGL);
    }
}


} // /namespace LLGL



// ================================================================================
