/*
 * GLRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "../CheckedCast.h"
#include "../Assertion.h"
#include "../../Core/Helper.h"
#include <LLGL/Desktop.h>
#include "RenderState/GLStateManager.h"
#include "GLTypes.h"
#include "Ext/GLExtensions.h"
#include "GLCore.h"
#include <sstream>


namespace LLGL
{


/* ----- Internal Functions ----- */

static void AssertCap(bool supported, const std::string& memberName)
{
    if (!supported)
    {
        /* Remove "has" from name */
        auto feature = memberName;
        if (feature.size() > 3 && feature.substr(0, 3) == "has")
            feature = feature.substr(3);

        /* Throw descriptive error */
        throw std::runtime_error(feature + " are not supported by the OpenGL renderer");
    }
}

#define LLGL_ASSERT_CAP(FEATURE) \
    AssertCap(renderingCaps_.FEATURE, #FEATURE)


/* ----- Render System ----- */

GLRenderSystem::GLRenderSystem()
{
}

GLRenderSystem::~GLRenderSystem()
{
    Desktop::ResetVideoMode();
}

std::map<RendererInfo, std::string> GLRenderSystem::QueryRendererInfo() const
{
    std::map<RendererInfo, std::string> info;

    std::vector<std::pair<RendererInfo, GLenum>> entries
    {{
        { RendererInfo::Version,                GL_VERSION                  },
        { RendererInfo::Vendor,                 GL_VENDOR                   },
        { RendererInfo::Hardware,               GL_RENDERER                 },
        { RendererInfo::ShadingLanguageVersion, GL_SHADING_LANGUAGE_VERSION },
    }};
    
    for (const auto& entry : entries)
    {
        auto bytes = glGetString(entry.second);
        if (bytes)
            info[entry.first] = std::string(reinterpret_cast<const char*>(bytes));
    }

    return info;
}

RenderingCaps GLRenderSystem::QueryRenderingCaps() const
{
    return renderingCaps_;
}

ShadingLanguage GLRenderSystem::QueryShadingLanguage() const
{
    /* Derive shading language version by OpenGL version */
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    auto IsVer = [major, minor](int maj, int min)
    {
        return (major == maj && minor == min);
    };

    if (IsVer(2, 0)) return ShadingLanguage::GLSL_110;
    if (IsVer(2, 1)) return ShadingLanguage::GLSL_120;
    if (IsVer(3, 0)) return ShadingLanguage::GLSL_130;
    if (IsVer(3, 1)) return ShadingLanguage::GLSL_140;
    if (IsVer(3, 2)) return ShadingLanguage::GLSL_150;
    if (IsVer(3, 3)) return ShadingLanguage::GLSL_330;
    if (IsVer(4, 0)) return ShadingLanguage::GLSL_400;
    if (IsVer(4, 1)) return ShadingLanguage::GLSL_410;
    if (IsVer(4, 2)) return ShadingLanguage::GLSL_420;
    if (IsVer(4, 3)) return ShadingLanguage::GLSL_430;
    if (IsVer(4, 4)) return ShadingLanguage::GLSL_440;
    if (IsVer(4, 5)) return ShadingLanguage::GLSL_450;

    return ShadingLanguage::GLSL_110;
}

/* ----- Render Context ----- */

RenderContext* GLRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    return AddRenderContext(MakeUnique<GLRenderContext>(*this, desc, window, nullptr), desc, window);
}

void GLRenderSystem::Release(RenderContext& renderContext)
{
    if (GetCurrentContext() == &renderContext)
        MakeCurrent(nullptr);
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Hardware Buffers ------ */

template <typename To, typename From>
GLHardwareBuffer& BindAndGetHardwareBuffer(From& buffer)
{
    auto& bufferGL = LLGL_CAST(To&, buffer);
    GLStateManager::active->BindBuffer(bufferGL);
    return bufferGL.hwBuffer;
}

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
    LLGL_ASSERT_CAP(hasConstantBuffers);
    return TakeOwnership(constantBuffers_, MakeUnique<GLConstantBuffer>());
}

StorageBuffer* GLRenderSystem::CreateStorageBuffer()
{
    LLGL_ASSERT_CAP(hasStorageBuffers);
    return TakeOwnership(storageBuffers_, MakeUnique<GLStorageBuffer>());
}

void GLRenderSystem::Release(VertexBuffer& vertexBuffer)
{
    RemoveFromUniqueSet(vertexBuffers_, &vertexBuffer);
}

void GLRenderSystem::Release(IndexBuffer& indexBuffer)
{
    RemoveFromUniqueSet(indexBuffers_, &indexBuffer);
}

void GLRenderSystem::Release(ConstantBuffer& constantBuffer)
{
    RemoveFromUniqueSet(constantBuffers_, &constantBuffer);
}

void GLRenderSystem::Release(StorageBuffer& storageBuffer)
{
    RemoveFromUniqueSet(storageBuffers_, &storageBuffer);
}

void GLRenderSystem::WriteVertexBuffer(
    VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat)
{
    /* Bind vertex buffer */
    auto& vertexBufferGL = LLGL_CAST(GLVertexBuffer&, vertexBuffer);
    GLStateManager::active->BindBuffer(vertexBufferGL);

    /* Update buffer data and update new vertex format */
    vertexBufferGL.hwBuffer.BufferData(data, dataSize, GLTypes::Map(usage));
    vertexBufferGL.UpdateVertexFormat(vertexFormat);
}

void GLRenderSystem::WriteIndexBuffer(
    IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat)
{
    /* Bind index buffer */
    auto& indexBufferGL = LLGL_CAST(GLIndexBuffer&, indexBuffer);
    GLStateManager::active->BindBuffer(indexBufferGL);

    /* Update buffer data and update new index format */
    indexBufferGL.hwBuffer.BufferData(data, dataSize, GLTypes::Map(usage));
    indexBufferGL.UpdateIndexFormat(indexFormat);
}

void GLRenderSystem::WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    BindAndGetHardwareBuffer<GLConstantBuffer>(constantBuffer).BufferData(data, dataSize, GLTypes::Map(usage));
}

void GLRenderSystem::WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    BindAndGetHardwareBuffer<GLStorageBuffer>(storageBuffer).BufferData(data, dataSize, GLTypes::Map(usage));
}

void GLRenderSystem::WriteVertexBufferSub(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    BindAndGetHardwareBuffer<GLVertexBuffer>(vertexBuffer).BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

void GLRenderSystem::WriteIndexBufferSub(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    BindAndGetHardwareBuffer<GLIndexBuffer>(indexBuffer).BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

void GLRenderSystem::WriteConstantBufferSub(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    BindAndGetHardwareBuffer<GLConstantBuffer>(constantBuffer).BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
}

void GLRenderSystem::WriteStorageBufferSub(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    BindAndGetHardwareBuffer<GLStorageBuffer>(storageBuffer).BufferSubData(data, dataSize, static_cast<GLintptr>(offset));
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

static void GLTexImageCube(GLenum internalFormat, int width, int height, AxisDirection cubeFace, GLenum format, GLenum type, const void* data)
{
    glTexImage2D(GLTypes::Map(cubeFace), 0, internalFormat, width, height, 0, format, type, data);
}

static void GLTexImage1DArray(GLenum internalFormat, int width, unsigned int layers, GLenum format, GLenum type, const void* data)
{
    glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, internalFormat, width, static_cast<GLsizei>(layers), 0, format, type, data);
}

static void GLTexImage2DArray(GLenum internalFormat, int width, int height, unsigned int layers, GLenum format, GLenum type, const void* data)
{
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat, width, height, static_cast<GLsizei>(layers), 0, format, type, data);
}

static void GLTexImageCubeArray(GLenum internalFormat, int width, int height, unsigned int layers, GLenum format, GLenum type, const void* data)
{
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, internalFormat, width, height, static_cast<GLsizei>(layers)*6, 0, format, type, data);
}

Texture* GLRenderSystem::CreateTexture()
{
    return TakeOwnership(textures_, MakeUnique<GLTexture>());
}

void GLRenderSystem::Release(Texture& texture)
{
    RemoveFromUniqueSet(textures_, &texture);
}

TextureDescriptor GLRenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    /* Bind texture */
    auto& textureGL = LLGL_CAST(const GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    /* Setup texture descriptor */
    TextureDescriptor desc;
    InitMemory(desc);

    desc.type = texture.GetType();

    /* Query texture size */
    auto target = GLTypes::Map(texture.GetType());

    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &desc.texture3DDesc.width);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &desc.texture3DDesc.height);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH, &desc.texture3DDesc.depth);

    return desc;
}

void GLRenderSystem::WriteTexture1D(Texture& texture, const TextureFormat format, int size, const ImageDataDescriptor* imageDesc)
{
    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture1D);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage1D(
            GLTypes::Map(format),
            size,
            GLTypes::Map(imageDesc->dataFormat),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(size);
        GLTexImage1D(GLTypes::Map(format), size, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::WriteTexture2D(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc)
{
    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture2D);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage2D(
            GLTypes::Map(format),
            size.x, size.y,
            GLTypes::Map(imageDesc->dataFormat),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(size.x*size.y);
        GLTexImage2D(GLTypes::Map(format), size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::WriteTexture3D(Texture& texture, const TextureFormat format, const Gs::Vector3i& size, const ImageDataDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(has3DTextures);

    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture3D);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage3D(
            GLTypes::Map(format),
            size.x, size.y, size.z,
            GLTypes::Map(imageDesc->dataFormat),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(size.x*size.y*size.z);
        GLTexImage3D(GLTypes::Map(format), size.x, size.y, size.z, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::WriteTextureCube(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(hasCubeTextures);

    const std::array<AxisDirection, 6> cubeFaces
    {
        AxisDirection::XPos,
        AxisDirection::XNeg,
        AxisDirection::YPos,
        AxisDirection::YNeg,
        AxisDirection::ZPos,
        AxisDirection::ZNeg
    };

    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::TextureCube);

    if (imageDesc)
    {
        /* Setup texture image cube-faces from descriptor */
        auto imageFace          = reinterpret_cast<const char*>(imageDesc->data);
        auto imageFaceStride    = size.x * size.y * ColorFormatSize(imageDesc->dataFormat) * DataTypeSize(imageDesc->dataType);

        for (auto face : cubeFaces)
        {
            GLTexImageCube(
                GLTypes::Map(format),
                size.x, size.y,
                face,
                GLTypes::Map(imageDesc->dataFormat),
                GLTypes::Map(imageDesc->dataType),
                imageFace
            );

            imageFace += imageFaceStride;
        }
    }
    else
    {
        /* Initialize texture image cube-faces with default color */
        auto image = GetDefaultTextureImageRGBAub(size.x*size.y);
        for (auto face : cubeFaces)
            GLTexImageCube(GLTypes::Map(format), size.x, size.y, face, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::WriteTexture1DArray(Texture& texture, const TextureFormat format, int size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);

    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture1DArray);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage1DArray(
            GLTypes::Map(format),
            size,
            layers,
            GLTypes::Map(imageDesc->dataFormat),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(size*static_cast<int>(layers));
        GLTexImage1DArray(GLTypes::Map(format), size, layers, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::WriteTexture2DArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);

    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture2DArray);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage2DArray(
            GLTypes::Map(format),
            size.x, size.y,
            layers,
            GLTypes::Map(imageDesc->dataFormat),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(size.x*size.y*static_cast<int>(layers));
        GLTexImage2DArray(GLTypes::Map(format), size.x, size.y, layers, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::WriteTextureCubeArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(hasCubeTextureArrays);

    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::TextureCubeArray);

    if (imageDesc)
    {
        /* Setup texture image cube-faces from descriptor */
        GLTexImageCubeArray(
            GLTypes::Map(format),
            size.x, size.y, layers,
            GLTypes::Map(imageDesc->dataFormat),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data
        );
    }
    else
    {
        /* Initialize texture image cube-faces with default color */
        auto image = GetDefaultTextureImageRGBAub(size.x*size.y*static_cast<int>(layers*6));
        GLTexImageCubeArray(GLTypes::Map(format), size.x, size.y, layers, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

static void GLTexSubImage1D(int mipLevel, int x, int width, const ImageDataDescriptor& imageDesc)
{
    glTexSubImage1D(
        GL_TEXTURE_1D, mipLevel, x, width,
        GLTypes::Map(imageDesc.dataFormat), GLTypes::Map(imageDesc.dataType), imageDesc.data
    );
}

static void GLTexSubImage2D(int mipLevel, int x, int y, int width, int height, const ImageDataDescriptor& imageDesc)
{
    glTexSubImage2D(
        GL_TEXTURE_2D, mipLevel, x, y, width, height,
        GLTypes::Map(imageDesc.dataFormat), GLTypes::Map(imageDesc.dataType), imageDesc.data
    );
}

static void GLTexSubImage3D(int mipLevel, int x, int y, int z, int width, int height, int depth, const ImageDataDescriptor& imageDesc)
{
    glTexSubImage3D(
        GL_TEXTURE_3D, mipLevel, x, y, z, width, height, depth,
        GLTypes::Map(imageDesc.dataFormat), GLTypes::Map(imageDesc.dataType), imageDesc.data
    );
}

static void GLTexSubImageCube(int mipLevel, int x, int y, int width, int height, AxisDirection cubeFace, const ImageDataDescriptor& imageDesc)
{
    glTexSubImage2D(
        GLTypes::Map(cubeFace), mipLevel, x, y, width, height,
        GLTypes::Map(imageDesc.dataFormat), GLTypes::Map(imageDesc.dataType), imageDesc.data
    );
}

static void GLTexSubImage1DArray(int mipLevel, int x, unsigned int layerOffset, int width, unsigned int layers, const ImageDataDescriptor& imageDesc)
{
    glTexSubImage2D(
        GL_TEXTURE_1D_ARRAY, mipLevel, x, static_cast<GLsizei>(layerOffset), width, static_cast<GLsizei>(layers),
        GLTypes::Map(imageDesc.dataFormat), GLTypes::Map(imageDesc.dataType), imageDesc.data
    );
}

static void GLTexSubImage2DArray(
    int mipLevel, int x, int y, unsigned int layerOffset, int width, int height, unsigned int layers, const ImageDataDescriptor& imageDesc)
{
    glTexSubImage3D(
        GL_TEXTURE_2D_ARRAY, mipLevel, x, y, static_cast<GLsizei>(layerOffset), width, height, static_cast<GLsizei>(layers),
        GLTypes::Map(imageDesc.dataFormat), GLTypes::Map(imageDesc.dataType), imageDesc.data
    );
}

static void GLTexSubImageCubeArray(
    int mipLevel, int x, int y, unsigned int layerOffset, AxisDirection cubeFaceOffset,
    int width, int height, unsigned int cubeFaces, const ImageDataDescriptor& imageDesc)
{
    glTexSubImage3D(
        GL_TEXTURE_CUBE_MAP_ARRAY, mipLevel,
        x, y, static_cast<GLsizei>(layerOffset + static_cast<int>(cubeFaceOffset))*6, width, height, static_cast<GLsizei>(cubeFaces),
        GLTypes::Map(imageDesc.dataFormat), GLTypes::Map(imageDesc.dataType), imageDesc.data
    );
}

void GLRenderSystem::WriteTexture1DSub(
    Texture& texture, int mipLevel, int position, int size, const ImageDataDescriptor& imageDesc)
{
    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);
    GLTexSubImage1D(mipLevel, position, size, imageDesc);
}

void GLRenderSystem::WriteTexture2DSub(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc)
{
    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);
    GLTexSubImage2D(mipLevel, position.x, position.y, size.x, size.y, imageDesc);
}

void GLRenderSystem::WriteTexture3DSub(
    Texture& texture, int mipLevel, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDataDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(has3DTextures);

    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImage3D(mipLevel, position.x, position.y, position.z, size.x, size.y, size.z, imageDesc);
}

void GLRenderSystem::WriteTextureCubeSub(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const AxisDirection cubeFace, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasCubeTextures);

    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImageCube(mipLevel, position.x, position.y, size.x, size.y, cubeFace, imageDesc);
}

void GLRenderSystem::WriteTexture1DArraySub(
    Texture& texture, int mipLevel, int position, unsigned int layerOffset,
    int size, unsigned int layers, const ImageDataDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);

    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImage1DArray(mipLevel, position, layerOffset, size, layers, imageDesc);
}

void GLRenderSystem::WriteTexture2DArraySub(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset,
    const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);

    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImage2DArray(mipLevel, position.x, position.y, layerOffset, size.x, size.y, layers, imageDesc);
}

void GLRenderSystem::WriteTextureCubeArraySub(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset, const AxisDirection cubeFaceOffset,
    const Gs::Vector2i& size, unsigned int cubeFaces, const ImageDataDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasCubeTextureArrays);

    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImageCubeArray(mipLevel, position.x, position.y, layerOffset, cubeFaceOffset, size.x, size.y, cubeFaces, imageDesc);
}

void GLRenderSystem::ReadTexture(const Texture& texture, int mipLevel, ColorFormat dataFormat, DataType dataType, void* data)
{
    LLGL_ASSERT_PTR(data);

    /* Bind texture */
    auto& textureGL = LLGL_CAST(const GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    /* Read image data from texture */
    glGetTexImage(
        GLTypes::Map(textureGL.GetType()),
        mipLevel,
        GLTypes::Map(dataFormat),
        GLTypes::Map(dataType),
        data
    );
}

/* ----- Sampler States ---- */

Sampler* GLRenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    LLGL_ASSERT_CAP(hasSamplers);
    auto sampler = MakeUnique<GLSampler>();
    sampler->SetDesc(desc);
    return TakeOwnership(samplers_, std::move(sampler));
}

void GLRenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Render Targets ----- */

RenderTarget* GLRenderSystem::CreateRenderTarget(unsigned int multiSamples)
{
    LLGL_ASSERT_CAP(hasRenderTargets);
    return TakeOwnership(renderTargets_, MakeUnique<GLRenderTarget>(multiSamples));
}

void GLRenderSystem::Release(RenderTarget& renderTarget)
{
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* GLRenderSystem::CreateShader(const ShaderType type)
{
    /* Validate rendering capabilities for required shader type */
    switch (type)
    {
        case ShaderType::Geometry:
            LLGL_ASSERT_CAP(hasGeometryShaders);
            break;
        case ShaderType::TessControl:
        case ShaderType::TessEvaluation:
            LLGL_ASSERT_CAP(hasTessellationShaders);
            break;
        case ShaderType::Compute:
            LLGL_ASSERT_CAP(hasComputeShaders);
            break;
    }

    /* Make and return shader object */
    return TakeOwnership(shaders_, MakeUnique<GLShader>(type));
}

ShaderProgram* GLRenderSystem::CreateShaderProgram()
{
    return TakeOwnership(shaderPrograms_, MakeUnique<GLShaderProgram>());
}

void GLRenderSystem::Release(Shader& shader)
{
    RemoveFromUniqueSet(shaders_, &shader);
}

void GLRenderSystem::Release(ShaderProgram& shaderProgram)
{
    RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* GLRenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return TakeOwnership(graphicsPipelines_, MakeUnique<GLGraphicsPipeline>(desc));
}

void GLRenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    RemoveFromUniqueSet(graphicsPipelines_, &graphicsPipeline);
}

/* ----- Queries ----- */

Query* GLRenderSystem::CreateQuery(const QueryType type)
{
    return TakeOwnership(queries_, MakeUnique<GLQuery>(type));
}

void GLRenderSystem::Release(Query& query)
{
    RemoveFromUniqueSet(queries_, &query);
}

/* ----- Extended Internal Functions ----- */

bool GLRenderSystem::HasExtension(const std::string& name) const
{
    return (extensionMap_.find(name) != extensionMap_.end());
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
        Desktop::SetVideoMode(desc.videoMode);

    /* Load all OpenGL extensions for the first time */
    if (renderContexts_.empty())
    {
        LoadGLExtensions(desc.profileOpenGL);
        SetDebugCallback(desc.debugCallback);
    }

    /* Use uniform clipping space */
    GLStateManager::active->SetClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);

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

        /* Query and store all rendering capabilities */
        StoreRenderingCaps();
    }
}

#ifdef LLGL_DEBUG

void APIENTRY GLDebugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    /* Generate output stream */
    std::stringstream typeStr;

    typeStr
        << "OpenGL debug callback ("
        << GLDebugSourceToStr(source) << ", "
        << GLDebugTypeToStr(type) << ", "
        << GLDebugSeverityToStr(severity) << ")";

    /* Call debug callback */
    auto& debugCallback = *reinterpret_cast<const DebugCallback*>(userParam);
    debugCallback(typeStr.str(), message);
}

#endif

void GLRenderSystem::SetDebugCallback(const DebugCallback& debugCallback)
{
    #if defined(LLGL_DEBUG) && !defined(__APPLE__)

    if (debugCallback)
    {
        GLStateManager::active->Enable(GLState::DEBUG_OUTPUT);
        GLStateManager::active->Enable(GLState::DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallback, &debugCallback);
    }
    else
    {
        GLStateManager::active->Disable(GLState::DEBUG_OUTPUT);
        GLStateManager::active->Disable(GLState::DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(nullptr, nullptr);
    }

    #endif
}

void GLRenderSystem::BindTextureAndSetType(GLTexture& textureGL, const TextureType type)
{
    if (textureGL.GetType() == type)
    {
        /* If type is already set, just bind texture */
        GLStateManager::active->BindTexture(textureGL);
    }
    else
    {
        /* If texture is not undefined -> recreate it */
        if (textureGL.GetType() != TextureType::Undefined)
            textureGL.Recreate();

        /* Set type of the specified texture for the first time */
        textureGL.SetType(type);
        GLStateManager::active->ForcedBindTexture(textureGL);

        /* Setup texture parameters for the first time */
        auto target = GLTypes::Map(type);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

void GLRenderSystem::StoreRenderingCaps()
{
    auto& caps = renderingCaps_;

    /* Set fixed states for this renderer */
    caps.screenOrigin                   = ScreenOrigin::LowerLeft;
    caps.clippingRange                  = ClippingRange::MinusOneToOne;

    /* Query all boolean capabilies by their respective OpenGL extension */
    caps.hasRenderTargets               = HasExtension("GL_ARB_framebuffer_object");
    caps.has3DTextures                  = HasExtension("GL_EXT_texture3D");
    caps.hasCubeTextures                = HasExtension("GL_ARB_texture_cube_map");
    caps.hasTextureArrays               = HasExtension("GL_EXT_texture_array");
    caps.hasCubeTextureArrays           = HasExtension("GL_ARB_texture_cube_map_array");
    caps.hasSamplers                    = HasExtension("GL_ARB_sampler_objects");
    caps.hasConstantBuffers             = HasExtension("GL_ARB_uniform_buffer_object");
    caps.hasStorageBuffers              = HasExtension("GL_ARB_shader_storage_buffer_object");
    caps.hasUniforms                    = HasExtension("GL_ARB_shader_objects");
    caps.hasGeometryShaders             = HasExtension("GL_ARB_geometry_shader4");
    caps.hasTessellationShaders         = HasExtension("GL_ARB_tessellation_shader");
    caps.hasComputeShaders              = HasExtension("GL_ARB_compute_shader");
    caps.hasInstancing                  = HasExtension("GL_ARB_draw_instanced");
    caps.hasOffsetInstancing            = HasExtension("GL_ARB_base_instance");
    caps.hasViewportArrays              = HasExtension("GL_ARB_viewport_array");
    caps.hasConservativeRasterization   = (HasExtension("GL_NV_conservative_raster") || HasExtension("GL_INTEL_conservative_rasterization"));

    /* Query integral attributes */
    auto GetUInt = [](GLenum param)
    {
        GLint attr = 0;
        glGetIntegerv(param, &attr);
        return static_cast<unsigned int>(attr);
    };

    auto GetUIntIdx = [](GLenum param, GLuint index)
    {
        GLint attr = 0;
        if (glGetIntegeri_v)
            glGetIntegeri_v(param, index, &attr);
        return static_cast<unsigned int>(attr);
    };

    caps.maxNumTextureArrayLayers           = GetUInt(GL_MAX_ARRAY_TEXTURE_LAYERS);
    caps.maxNumRenderTargetAttachments      = GetUInt(GL_MAX_DRAW_BUFFERS);
    caps.maxConstantBufferSize              = GetUInt(GL_MAX_UNIFORM_BLOCK_SIZE);
    caps.maxAnisotropy                      = 16;
    
    caps.maxNumComputeShaderWorkGroups.x    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0);
    caps.maxNumComputeShaderWorkGroups.y    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1);
    caps.maxNumComputeShaderWorkGroups.z    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2);

    caps.maxComputeShaderWorkGroupSize.x    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0);
    caps.maxComputeShaderWorkGroupSize.y    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1);
    caps.maxComputeShaderWorkGroupSize.z    = GetUIntIdx(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2);

    /* Query maximum texture dimensions */
    GLint querySizeBase = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &querySizeBase);

    /* Query 1D texture max size */
    auto querySize = querySizeBase;

    while (caps.max1DTextureSize == 0 && querySize > 0)
    {
        glTexImage1D(GL_PROXY_TEXTURE_1D, 0, GL_RGBA, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_1D, 0, GL_TEXTURE_WIDTH, &(caps.max1DTextureSize));
        querySize /= 2;
    }

    /* Query 2D texture max size */
    querySize = querySizeBase;

    while (caps.max2DTextureSize == 0 && querySize > 0)
    {
        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &(caps.max2DTextureSize));
        querySize /= 2;
    }

    /* Query 3D texture max size */
    if (caps.has3DTextures)
    {
        querySize = querySizeBase;

        while (caps.max3DTextureSize == 0 && querySize > 0)
        {
            glTexImage3D(GL_PROXY_TEXTURE_3D, 0, GL_RGBA, querySize, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &(caps.max3DTextureSize));
            querySize /= 2;
        }
    }

    /* Query cube texture max size */
    if (caps.hasCubeTextures)
    {
        querySize = querySizeBase;

        while (caps.maxCubeTextureSize == 0 && querySize > 0)
        {
            glTexImage2D(GL_PROXY_TEXTURE_CUBE_MAP, 0, GL_RGBA, querySize, querySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glGetTexLevelParameteriv(GL_PROXY_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_WIDTH, &(caps.maxCubeTextureSize));
            querySize /= 2;
        }
    }
}


#undef LLGL_ASSERT_CAP


} // /namespace LLGL



// ================================================================================
