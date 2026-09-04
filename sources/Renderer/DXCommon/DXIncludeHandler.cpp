/*
 * DXIncludeHandler.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DXIncludeHandler.h"
#include "../../Core/Assertion.h"
#include <d3dcompiler.h>


namespace LLGL
{


DXIncludeHandler::DXIncludeHandler(IncludeHandler* forwardIncludeHandler, Report& outReport) :
    outReport_             { outReport             },
    forwardIncludeHandler_ { forwardIncludeHandler }
{
}

HRESULT DXIncludeHandler::Open(
    D3D_INCLUDE_TYPE    /*includeType*/,
    LPCSTR              filename,
    LPCVOID             /*parentData*/,
    LPCVOID*            outData,
    UINT*               outDataSize)
{
    LLGL_ASSERT_PTR(forwardIncludeHandler_);

    if (forwardIncludeHandler_->Include(filename, fileContent_, outReport_))
    {
        *outData        = fileContent_.GetData();
        *outDataSize    = static_cast<UINT>(fileContent_.GetSize());
        return S_OK;
    }

    return E_FAIL;
}

HRESULT DXIncludeHandler::Close(LPCVOID data)
{
    if (forwardIncludeHandler_ == nullptr)
        return E_POINTER;
    fileContent_ = {};
    return S_OK;
}

ID3DInclude* DXIncludeHandler::GetSelfOrDefault()
{
    return (forwardIncludeHandler_ != nullptr ? this : D3D_COMPILE_STANDARD_FILE_INCLUDE);
}


} // /namespace LLGL



// ================================================================================
