/*
 * DXBlob.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DXBlob.h"
#include <utility>


namespace LLGL
{


std::unique_ptr<Blob> DXBlob::CreateWrapper(ComPtr<ID3DBlob>&& native)
{
    return (native ? std::unique_ptr<DXBlob>(new DXBlob(std::forward<ComPtr<ID3DBlob>>(native))) : nullptr);
}

std::unique_ptr<Blob> DXBlob::CreateWrapper(const ComPtr<ID3DBlob>& native)
{
    return (native ? std::unique_ptr<DXBlob>(new DXBlob(native)) : nullptr);
}

const void* DXBlob::GetData() const
{
    return native_->GetBufferPointer();
}

std::size_t DXBlob::GetSize() const
{
    return native_->GetBufferSize();
}


/*
 * ======= Private: =======
 */

DXBlob::DXBlob(ComPtr<ID3DBlob>&& native) :
    native_ { std::move(native) }
{
}

DXBlob::DXBlob(const ComPtr<ID3DBlob>& native) :
    native_ { native }
{
}


} // /namespace LLGL



// ================================================================================
