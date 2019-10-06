/*
 * DXBlob.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DX_BLOB_H
#define LLGL_DX_BLOB_H


#include <LLGL/Blob.h>
#include <d3dcommon.h>
#include "ComPtr.h"


namespace LLGL
{


// <ID3DBlob> wrapper implementation of <Blob> interface.
class DXBlob final : public Blob
{

    public:

        // Creates a wrapped Blob instance for an ID3DBlob object.
        static std::unique_ptr<Blob> CreateWrapper(ComPtr<ID3DBlob>&& native);
        static std::unique_ptr<Blob> CreateWrapper(const ComPtr<ID3DBlob>& native);

    public:

        const void* GetData() const override;
        std::size_t GetSize() const override;

    private:

        DXBlob(ComPtr<ID3DBlob>&& native);
        DXBlob(const ComPtr<ID3DBlob>& native);

    private:

        ComPtr<ID3DBlob> native_;

};


} // /namespace LLGL


#endif



// ================================================================================
