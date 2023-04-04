/*
 * DXBlob.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
