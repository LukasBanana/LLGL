/*
 * DXIncludeHandler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DX_INCLUDE_HANDLER_H
#define LLGL_DX_INCLUDE_HANDLER_H


#include <LLGL/IncludeHandler.h>
#include <LLGL/Report.h>
#include <d3dcommon.h>


namespace LLGL
{


// Wraps the LLGL::IncludeHandler interface for use with the ID3DInclude interface.
class DXIncludeHandler final : public ID3DInclude
{

    public:

        DXIncludeHandler(IncludeHandler* forwardIncludeHandler, Report& outReport);

        STDMETHOD(Open)(
            D3D_INCLUDE_TYPE    includeType,
            LPCSTR              filename,
            LPCVOID             parentData,
            LPCVOID*            outData,
            UINT*               outDataSize
        ) override;

        STDMETHOD(Close)(LPCVOID data) override;

        ID3DInclude* GetSelfOrDefault();

    private:

        Report&         outReport_;
        IncludeHandler* forwardIncludeHandler_ = nullptr;
        Blob            fileContent_;

};



} // /namespace LLGL


#endif



// ================================================================================
