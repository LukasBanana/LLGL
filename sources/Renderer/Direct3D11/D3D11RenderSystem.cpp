/*
 * D3D11RenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderSystem.h"
#include "D3D11Types.h"
#include "../DXCommon/DXCore.h"
#include "../CheckedCast.h"
#include "../Assertion.h"
#include "../../Core/Helper.h"
#include "../../Core/Vendor.h"


namespace LLGL
{


D3D11RenderSystem::D3D11RenderSystem()
{
    /* Create DXGU factory, query video adapters, and create D3D11 device */
    CreateFactory();
    //QueryVideoAdapters();
    CreateDevice(nullptr);
    InitStateManager();
}

D3D11RenderSystem::~D3D11RenderSystem()
{
}

std::map<RendererInfo, std::string> D3D11RenderSystem::QueryRendererInfo() const
{
    std::map<RendererInfo, std::string> info;

    //todo

    return info;
}

RenderingCaps D3D11RenderSystem::QueryRenderingCaps() const
{
    RenderingCaps caps;
    DXGetRenderingCaps(caps, GetFeatureLevel());
    return caps;
}

ShadingLanguage D3D11RenderSystem::QueryShadingLanguage() const
{
    return DXGetHLSLVersion(GetFeatureLevel());
}

/* ----- Render Context ----- */

RenderContext* D3D11RenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    /* Create new render context and make it the current one */
    auto renderContext = MakeUnique<D3D11RenderContext>(*this, *stateMngr_, context_, desc, window);
    MakeCurrent(renderContext.get());

    /*
    If render context created it's own window then show it after creation,
    since anti-aliasing may force the window to be recreated several times
    */
    if (!window)
        renderContext->GetWindow().Show();

    /* Take ownership and return new render context */
    return TakeOwnership(renderContexts_, std::move(renderContext));
}

void D3D11RenderSystem::Release(RenderContext& renderContext)
{
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Hardware Buffers ------ */

VertexBuffer* D3D11RenderSystem::CreateVertexBuffer()
{
    return TakeOwnership(vertexBuffers_, MakeUnique<D3D11VertexBuffer>());
}

IndexBuffer* D3D11RenderSystem::CreateIndexBuffer()
{
    return TakeOwnership(indexBuffers_, MakeUnique<D3D11IndexBuffer>());
}

ConstantBuffer* D3D11RenderSystem::CreateConstantBuffer()
{
    return TakeOwnership(constantBuffers_, MakeUnique<D3D11ConstantBuffer>());
}

StorageBuffer* D3D11RenderSystem::CreateStorageBuffer()
{
    return nullptr;//TakeOwnership(storageBuffers_, MakeUnique<D3D11StorageBuffer>());
}

void D3D11RenderSystem::Release(VertexBuffer& vertexBuffer)
{
    RemoveFromUniqueSet(vertexBuffers_, &vertexBuffer);
}

void D3D11RenderSystem::Release(IndexBuffer& indexBuffer)
{
    RemoveFromUniqueSet(indexBuffers_, &indexBuffer);
}

void D3D11RenderSystem::Release(ConstantBuffer& constantBuffer)
{
    RemoveFromUniqueSet(constantBuffers_, &constantBuffer);
}

void D3D11RenderSystem::Release(StorageBuffer& storageBuffer)
{
    //RemoveFromUniqueSet(storageBuffers_, &storageBuffer);
}

void D3D11RenderSystem::SetupVertexBuffer(
    VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat)
{
    auto& vertexBufferD3D = LLGL_CAST(D3D11VertexBuffer&, vertexBuffer);
    vertexBufferD3D.CreateResource(device_.Get(), vertexFormat.GetFormatSize(), dataSize, data);
}

void D3D11RenderSystem::SetupIndexBuffer(
    IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat)
{
    auto& indexBufferD3D = LLGL_CAST(D3D11IndexBuffer&, indexBuffer);
    indexBufferD3D.CreateResource(device_.Get(), D3D11Types::Map(indexFormat.GetDataType()), dataSize, data);
}

void D3D11RenderSystem::SetupConstantBuffer(
    ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    auto& constantBufferD3D = LLGL_CAST(D3D11ConstantBuffer&, constantBuffer);
    constantBufferD3D.CreateResource(device_.Get(), dataSize, usage, data);
}

void D3D11RenderSystem::SetupStorageBuffer(
    StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    //todo
}

void D3D11RenderSystem::WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    auto& vertexBufferD3D = LLGL_CAST(D3D11VertexBuffer&, vertexBuffer);
    vertexBufferD3D.hwBuffer.UpdateSubresource(context_.Get(), data, static_cast<UINT>(dataSize), static_cast<UINT>(offset));
}

void D3D11RenderSystem::WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    auto& indexBufferD3D = LLGL_CAST(D3D11IndexBuffer&, indexBuffer);
    indexBufferD3D.hwBuffer.UpdateSubresource(context_.Get(), data, static_cast<UINT>(dataSize), static_cast<UINT>(offset));
}

void D3D11RenderSystem::WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    auto& constantBufferD3D = LLGL_CAST(D3D11ConstantBuffer&, constantBuffer);
    constantBufferD3D.UpdateSubresource(context_.Get(), data, static_cast<UINT>(dataSize), static_cast<UINT>(offset));
}

void D3D11RenderSystem::WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    //todo
}

/* ----- Textures ----- */

Texture* D3D11RenderSystem::CreateTexture()
{
    return TakeOwnership(textures_, MakeUnique<D3D11Texture>());
}

void D3D11RenderSystem::Release(Texture& texture)
{
    RemoveFromUniqueSet(textures_, &texture);
}

TextureDescriptor D3D11RenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    /* Get D3D hardware texture resource */
    auto& textureD3D = LLGL_CAST(const D3D11Texture&, texture);
    const auto& hwTex = textureD3D.GetHardwareTexture();

    /* Initialize texture descriptor */
    TextureDescriptor texDesc;
    InitMemory(texDesc);

    texDesc.type = texture.GetType();

    /* Get resource dimension to query the respective D3D descriptor */
    D3D11_RESOURCE_DIMENSION dimension;
    hwTex.resource->GetType(&dimension);

    switch (dimension)
    {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            /* Query descriptor from 1D texture */
            D3D11_TEXTURE1D_DESC desc;
            hwTex.tex1D->GetDesc(&desc);

            texDesc.texture1DDesc.width     = static_cast<int>(desc.Width);
            texDesc.texture1DDesc.layers    = desc.ArraySize;
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            /* Query descriptor from 2D texture */
            D3D11_TEXTURE2D_DESC desc;
            hwTex.tex2D->GetDesc(&desc);

            texDesc.texture2DDesc.width     = static_cast<int>(desc.Width);
            texDesc.texture2DDesc.height    = static_cast<int>(desc.Height);
            texDesc.texture2DDesc.layers    = desc.ArraySize;
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            /* Query descriptor from 3D texture */
            D3D11_TEXTURE3D_DESC desc;
            hwTex.tex3D->GetDesc(&desc);

            texDesc.texture3DDesc.width     = static_cast<int>(desc.Width);
            texDesc.texture3DDesc.height    = static_cast<int>(desc.Height);
            texDesc.texture3DDesc.depth     = static_cast<int>(desc.Depth);
        }
        break;
    }

    return texDesc;
}

void D3D11RenderSystem::SetupTexture1D(Texture& texture, const TextureFormat format, int size, const ImageDescriptor* imageDesc)
{
    /* Get D3D texture, set type, and create generic 1D-texture */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    textureD3D.SetType(TextureType::Texture1D);
    SetupGenericTexture1D(textureD3D, format, size, 1, imageDesc, D3D11_RESOURCE_MISC_GENERATE_MIPS);
}

void D3D11RenderSystem::SetupTexture2D(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDescriptor* imageDesc)
{
    /* Get D3D texture, set type, and create generic 2D-texture */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    textureD3D.SetType(TextureType::Texture2D);
    SetupGenericTexture2D(textureD3D, format, size, 1, imageDesc, D3D11_RESOURCE_MISC_GENERATE_MIPS);
}

void D3D11RenderSystem::SetupTexture3D(Texture& texture, const TextureFormat format, const Gs::Vector3i& size, const ImageDescriptor* imageDesc)
{
    /* Get D3D texture, set type, and create generic 1D-texture */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    textureD3D.SetType(TextureType::Texture3D);
    SetupGenericTexture3D(textureD3D, format, size, imageDesc, D3D11_RESOURCE_MISC_GENERATE_MIPS);
}

void D3D11RenderSystem::SetupTextureCube(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDescriptor* imageDesc)
{
    /* Get D3D texture, set type, and create generic 2D-texture */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    textureD3D.SetType(TextureType::TextureCube);
    SetupGenericTexture2D(textureD3D, format, size, 6, imageDesc, D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE);
}

void D3D11RenderSystem::SetupTexture1DArray(Texture& texture, const TextureFormat format, int size, unsigned int layers, const ImageDescriptor* imageDesc)
{
    /* Get D3D texture, set type, and create generic 1D-texture */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    textureD3D.SetType(TextureType::Texture1DArray);
    SetupGenericTexture1D(textureD3D, format, size, layers, imageDesc, D3D11_RESOURCE_MISC_GENERATE_MIPS);
}

void D3D11RenderSystem::SetupTexture2DArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDescriptor* imageDesc)
{
    /* Get D3D texture, set type, and create generic 2D-texture */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    textureD3D.SetType(TextureType::Texture2DArray);
    SetupGenericTexture2D(textureD3D, format, size, layers, imageDesc, D3D11_RESOURCE_MISC_GENERATE_MIPS);
}

void D3D11RenderSystem::SetupTextureCubeArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDescriptor* imageDesc)
{
    /* Get D3D texture, set type, and create generic 2D-texture */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    textureD3D.SetType(TextureType::TextureCubeArray);
    SetupGenericTexture2D(textureD3D, format, size, layers*6, imageDesc, D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE);
}

void D3D11RenderSystem::WriteTexture1D(
    Texture& texture, int mipLevel, int position, int size, const ImageDescriptor& imageDesc)
{
    UpdateGenerateTexture(texture, mipLevel, 0, { position, 0, 0 }, { size, 1, 1 }, imageDesc);
}

void D3D11RenderSystem::WriteTexture2D(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const Gs::Vector2i& size, const ImageDescriptor& imageDesc)
{
    UpdateGenerateTexture(texture, mipLevel, 0, { position.x, position.y, 0 }, { size.x, size.y, 1 }, imageDesc);
}

void D3D11RenderSystem::WriteTexture3D(
    Texture& texture, int mipLevel, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDescriptor& imageDesc)
{
    UpdateGenerateTexture(texture, mipLevel, 0, position, size, imageDesc);
}

void D3D11RenderSystem::WriteTextureCube(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const AxisDirection cubeFace, const Gs::Vector2i& size, const ImageDescriptor& imageDesc)
{
    UpdateGenerateTexture(texture, mipLevel, static_cast<UINT>(cubeFace), { position.x, position.y, 0 }, { size.x, size.y, 1 }, imageDesc);
}

void D3D11RenderSystem::WriteTexture1DArray(
    Texture& texture, int mipLevel, int position, unsigned int layerOffset,
    int size, unsigned int layers, const ImageDescriptor& imageDesc)
{
    UpdateGenerateTexture(texture, mipLevel, layerOffset, { position, 0, 0 }, { size, static_cast<int>(layers), 1 }, imageDesc);
}

void D3D11RenderSystem::WriteTexture2DArray(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset,
    const Gs::Vector2i& size, unsigned int layers, const ImageDescriptor& imageDesc)
{
    UpdateGenerateTexture(texture, mipLevel, layerOffset, { position.x, position.y, 0 }, { size.x, size.y, static_cast<int>(layers) }, imageDesc);
}

void D3D11RenderSystem::WriteTextureCubeArray(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset, const AxisDirection cubeFaceOffset,
    const Gs::Vector2i& size, unsigned int cubeFaces, const ImageDescriptor& imageDesc)
{
    UpdateGenerateTexture(texture, mipLevel, layerOffset*6, { position.x, position.y, 0 }, { size.x, size.y, static_cast<int>(cubeFaces) }, imageDesc);
}

void D3D11RenderSystem::ReadTexture(const Texture& texture, int mipLevel, ImageFormat dataFormat, DataType dataType, void* data)
{
    LLGL_ASSERT_PTR(data);

    //todo...
}

/* ----- Sampler States ---- */

Sampler* D3D11RenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return TakeOwnership(samplers_, MakeUnique<D3D11Sampler>(device_.Get(), desc));
}

void D3D11RenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Render Targets ----- */

RenderTarget* D3D11RenderSystem::CreateRenderTarget(unsigned int multiSamples)
{
    return TakeOwnership(renderTargets_, MakeUnique<D3D11RenderTarget>(device_.Get(), multiSamples));
}

void D3D11RenderSystem::Release(RenderTarget& renderTarget)
{
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* D3D11RenderSystem::CreateShader(const ShaderType type)
{
    return TakeOwnership(shaders_, MakeUnique<D3D11Shader>(device_.Get(), type));
}

ShaderProgram* D3D11RenderSystem::CreateShaderProgram()
{
    return TakeOwnership(shaderPrograms_, MakeUnique<D3D11ShaderProgram>(device_.Get()));
}

void D3D11RenderSystem::Release(Shader& shader)
{
    RemoveFromUniqueSet(shaders_, &shader);
}

void D3D11RenderSystem::Release(ShaderProgram& shaderProgram)
{
    RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* D3D11RenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return TakeOwnership(graphicsPipelines_, MakeUnique<D3D11GraphicsPipeline>(device_.Get(), desc));
}

ComputePipeline* D3D11RenderSystem::CreateComputePipeline(const ComputePipelineDescriptor& desc)
{
    return TakeOwnership(computePipelines_, MakeUnique<D3D11ComputePipeline>(desc));
}

void D3D11RenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    RemoveFromUniqueSet(graphicsPipelines_, &graphicsPipeline);
}

void D3D11RenderSystem::Release(ComputePipeline& computePipeline)
{
    RemoveFromUniqueSet(computePipelines_, &computePipeline);
}

/* ----- Queries ----- */

Query* D3D11RenderSystem::CreateQuery(const QueryType type)
{
    return TakeOwnership(queries_, MakeUnique<D3D11Query>(device_.Get(), type));
}

void D3D11RenderSystem::Release(Query& query)
{
    RemoveFromUniqueSet(queries_, &query);
}


/* ----- Extended internal functions ----- */

ComPtr<IDXGISwapChain> D3D11RenderSystem::CreateDXSwapChain(DXGI_SWAP_CHAIN_DESC& desc)
{
    ComPtr<IDXGISwapChain> swapChain;

    auto hr = factory_->CreateSwapChain(device_.Get(), &desc, &swapChain);
    DXThrowIfFailed(hr, "failed to create D3D11 swap chain");

    return swapChain;
}


/*
 * ======= Private: =======
 */

void D3D11RenderSystem::CreateFactory()
{
    /* Create DXGI factory */
    auto hr = CreateDXGIFactory(IID_PPV_ARGS(&factory_));
    DXThrowIfFailed(hr, "failed to create DXGI factor");
}

//TODO -> generalize this for D3D11 and D3D12
/*void D3D11RenderSystem::QueryVideoAdapters()
{
}*/

void D3D11RenderSystem::CreateDevice(IDXGIAdapter* adapter)
{
    /* Use default adapter (null) and try all feature levels */
    auto    featureLevels   = DXGetFeatureLevels(D3D_FEATURE_LEVEL_11_1);
    HRESULT hr              = 0;
    UINT    flags           = 0;

    #ifdef LLGL_DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    for (D3D_DRIVER_TYPE driver : { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE })
    {
        auto hr = D3D11CreateDevice(
            adapter,                // Video adapter
            driver,                 // Driver type
            0,                      // Software rasterizer module (none)
            flags,                  // Flags
            featureLevels.data(),   // Feature level
            featureLevels.size(),   // Num feature levels
            D3D11_SDK_VERSION,      // SDK version
            &device_,               // Output device
            &featureLevel_,         // Output feature level
            &context_               // Output device context
        );

        if (SUCCEEDED(hr))
            break;
    }

    DXThrowIfFailed(hr, "failed to create D3D11 device");
}

void D3D11RenderSystem::InitStateManager()
{
    /* Create state manager */
    stateMngr_ = MakeUnique<D3D11StateManager>(context_);
}

void D3D11RenderSystem::SetupGenericTexture1D(
    D3D11Texture& textureD3D, const TextureFormat format, int size, unsigned int layers, const ImageDescriptor* imageDesc, UINT miscFlags)
{
    /* Setup D3D texture descriptor */
    D3D11_TEXTURE1D_DESC texDesc;
    {
        texDesc.Width           = static_cast<UINT>(size);
        texDesc.MipLevels       = 0;
        texDesc.ArraySize       = layers;
        texDesc.Format          = D3D11Types::Map(format);
        texDesc.Usage           = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags       = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        texDesc.CPUAccessFlags  = 0;
        texDesc.MiscFlags       = miscFlags;
    }

    /* Create D3D texture resource */
    textureD3D.CreateTexture1D(device_.Get(), texDesc);

    if (imageDesc)
        textureD3D.UpdateSubresource(context_.Get(), 0, 0, CD3D11_BOX(0, 0, 0, size, layers, 1), *imageDesc, GetConfiguration().threadCount);
    else
    {
        //TODO -> fill texture with default data
    }
}

void D3D11RenderSystem::SetupGenericTexture2D(
    D3D11Texture& textureD3D, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDescriptor* imageDesc, UINT miscFlags)
{
    /* Setup D3D texture descriptor */
    D3D11_TEXTURE2D_DESC texDesc;
    {
        texDesc.Width               = static_cast<UINT>(size.x);
        texDesc.Height              = static_cast<UINT>(size.y);
        texDesc.MipLevels           = 0;
        texDesc.ArraySize           = layers;
        texDesc.Format              = D3D11Types::Map(format);
        texDesc.SampleDesc.Count    = 1;
        texDesc.SampleDesc.Quality  = 0;
        texDesc.Usage               = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        texDesc.CPUAccessFlags      = 0;
        texDesc.MiscFlags           = miscFlags;
    }

    /* Create D3D texture resource */
    textureD3D.CreateTexture2D(device_.Get(), texDesc);

    if (imageDesc)
        textureD3D.UpdateSubresource(context_.Get(), 0, 0, CD3D11_BOX(0, 0, 0, size.x, size.y, layers), *imageDesc, GetConfiguration().threadCount);
    else
    {
        //TODO -> fill texture with default data
    }
}

void D3D11RenderSystem::SetupGenericTexture3D(
    D3D11Texture& textureD3D, const TextureFormat format, const Gs::Vector3i& size, const ImageDescriptor* imageDesc, UINT miscFlags)
{
    /* Setup D3D texture descriptor */
    D3D11_TEXTURE3D_DESC texDesc;
    {
        texDesc.Width           = static_cast<UINT>(size.x);
        texDesc.Height          = static_cast<UINT>(size.y);
        texDesc.Depth           = static_cast<UINT>(size.z);
        texDesc.MipLevels       = 0;
        texDesc.Format          = D3D11Types::Map(format);
        texDesc.Usage           = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags       = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        texDesc.CPUAccessFlags  = 0;
        texDesc.MiscFlags       = miscFlags;
    }

    /* Create D3D texture resource */
    textureD3D.CreateTexture3D(device_.Get(), texDesc);

    if (imageDesc)
        textureD3D.UpdateSubresource(context_.Get(), 0, 0, CD3D11_BOX(0, 0, 0, size.x, size.y, size.z), *imageDesc, GetConfiguration().threadCount);
    else
    {
        //TODO -> fill texture with default data
    }
}

void D3D11RenderSystem::UpdateGenerateTexture(
    Texture& texture, int mipLevel, unsigned int layer, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDescriptor& imageDesc)
{
    /* Get D3D texture and update subresource */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    textureD3D.UpdateSubresource(
        context_.Get(), static_cast<UINT>(mipLevel), layer,
        CD3D11_BOX(
            position.x, position.y, position.z,
            position.x + size.x, position.y + size.y, position.z + size.z
        ),
        imageDesc, GetConfiguration().threadCount
    );
}


} // /namespace LLGL



// ================================================================================
