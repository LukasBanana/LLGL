/*
 * D3D11BufferWithRV.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11BufferWithRV.h"
#include "../D3D11Types.h"
#include "../D3D11ObjectUtils.h"
#include "../../DXCommon/DXCore.h"
#include "../../BufferUtils.h"
#include "../../../Core/Helper.h"
#include "../../../Core/Assertion.h"
#include <algorithm>


namespace LLGL
{


static DXGI_FORMAT GetD3DResourceViewFormat(const BufferDescriptor& desc)
{
    /*
    D3D11_BUFFER_UAV_FLAG_RAW buffer flag for ByteAddressBuffer requires the UAV to have the DXGI_FORMAT_R32_TYPELESS format.
    -> see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476096(v=vs.85).aspx
    */
    if (IsTypedBuffer(desc))
        return D3D11Types::Map(desc.format);
    if (IsByteAddressBuffer(desc))
        return DXGI_FORMAT_R32_TYPELESS;
    return DXGI_FORMAT_UNKNOWN;
}

static UINT GetUAVFlags(const BufferDescriptor& desc)
{
    if ((desc.bindFlags & BindFlags::Storage) != 0)
    {
        if (IsStructuredBuffer(desc))
        {
            if ((desc.miscFlags & MiscFlags::Append) != 0)
                return D3D11_BUFFER_UAV_FLAG_APPEND;
            if ((desc.miscFlags & MiscFlags::Counter) != 0)
                return D3D11_BUFFER_UAV_FLAG_COUNTER;
        }
        else if (IsByteAddressBuffer(desc))
            return D3D11_BUFFER_UAV_FLAG_RAW;
    }
    return 0;
}

D3D11BufferWithRV::D3D11BufferWithRV(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData) :
    D3D11Buffer   { device, desc, initialData },
    uavFlags_     { GetUAVFlags(desc)         }
{
    /* Determine stride size either for structured buffers or regular buffers */
    const UINT stride = GetStorageBufferStride(desc);

    /* Create resource views (SRV and UAV) */
    auto format         = GetD3DResourceViewFormat(desc);
    auto numElements    = static_cast<UINT>(desc.size) / stride;

    if ((desc.bindFlags & BindFlags::Sampled) != 0)
        CreateInternalSRV(device, format, 0, numElements);

    if ((desc.bindFlags & BindFlags::Storage) != 0)
        CreateInternalUAV(device, format, 0, numElements);
}

void D3D11BufferWithRV::SetName(const char* name)
{
    D3D11Buffer::SetName(name);
    if (srv_)
        D3D11SetObjectNameSubscript(srv_.Get(), name, ".SRV");
    if (uav_)
        D3D11SetObjectNameSubscript(uav_.Get(), name, ".UAV");
}

static void CreateD3D11BufferSubresourceSRV(
    ID3D11Device*               device,
    ID3D11Resource*             resource,
    ID3D11ShaderResourceView**  srvOutput,
    DXGI_FORMAT                 format,
    UINT                        firstElement,
    UINT                        numElements,
    const char*                 errorContextInfo = nullptr)
{
    /* Initialize descriptor and create SRV */
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    {
        desc.Format                 = format;
        desc.ViewDimension          = D3D11_SRV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement    = firstElement;
        desc.Buffer.NumElements     = numElements;
    }
    auto hr = device->CreateShaderResourceView(resource, &desc, srvOutput);
    DXThrowIfCreateFailed(hr, "ID3D11ShaderResourceView", errorContextInfo);
}

void D3D11BufferWithRV::CreateSubresourceSRV(
    ID3D11Device*               device,
    ID3D11ShaderResourceView**  srvOutput,
    DXGI_FORMAT                 format,
    UINT                        firstElement,
    UINT                        numElements)
{
    CreateD3D11BufferSubresourceSRV(
        device,
        GetNative(),
        srvOutput,
        format,
        firstElement,
        numElements,
        "for buffer subresource"
    );
}

static void CreateD3D11BufferSubresourceUAV(
    ID3D11Device*               device,
    ID3D11Resource*             resource,
    ID3D11UnorderedAccessView** uavOutput,
    DXGI_FORMAT                 format,
    UINT                        firstElement,
    UINT                        numElements,
    UINT                        flags,
    const char*                 errorContextInfo = nullptr)
{
    /* Initialize descriptor and create UAV */
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    {
        desc.Format                 = format;
        desc.ViewDimension          = D3D11_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement    = firstElement;
        desc.Buffer.NumElements     = numElements;
        desc.Buffer.Flags           = flags;
    }
    auto hr = device->CreateUnorderedAccessView(resource, &desc, uavOutput);
    DXThrowIfCreateFailed(hr, "ID3D11UnorderedAccessView", errorContextInfo);
}

void D3D11BufferWithRV::CreateSubresourceUAV(
    ID3D11Device*               device,
    ID3D11UnorderedAccessView** uavOutput,
    DXGI_FORMAT                 format,
    UINT                        firstElement,
    UINT                        numElements)
{
    CreateD3D11BufferSubresourceUAV(
        device,
        GetNative(),
        uavOutput,
        format,
        firstElement,
        numElements,
        uavFlags_,
        "for buffer subresource"
    );
}


/*
 * ======= Private: =======
 */

void D3D11BufferWithRV::CreateInternalSRV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements)
{
    CreateD3D11BufferSubresourceSRV(
        device,
        GetNative(),
        srv_.ReleaseAndGetAddressOf(),
        format,
        firstElement,
        numElements,
        "for buffer"
    );
}

void D3D11BufferWithRV::CreateInternalUAV(ID3D11Device* device, DXGI_FORMAT format, UINT firstElement, UINT numElements)
{
    CreateD3D11BufferSubresourceUAV(
        device,
        GetNative(),
        uav_.ReleaseAndGetAddressOf(),
        format,
        firstElement,
        numElements,
        uavFlags_,
        "for buffer"
    );
}


} // /namespace LLGL



// ================================================================================
