/*
 * D3D11StorageBufferArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_STORAGE_BUFFER_ARRAY_H
#define LLGL_D3D11_STORAGE_BUFFER_ARRAY_H


#include "D3D11BufferArray.h"


namespace LLGL
{


class D3D11StorageBufferArray : public D3D11BufferArray
{

    public:

        D3D11StorageBufferArray(unsigned int numBuffers, Buffer* const * bufferArray);

        // True, if this storage buffer array has UAV objects.
        bool HasUAV() const;

        // Returns the array of UAV objects.
        inline const std::vector<ID3D11UnorderedAccessView*>& GetUnorderedViews() const
        {
            return unorderedViews_;
        }

        //! Returns the array of SRV objects.
        inline const std::vector<ID3D11ShaderResourceView*>& GetResourceViews() const
        {
            return resourceViews_;
        }

        // Returns the array of initial counts.
        inline const std::vector<UINT>& GetInitialCounts() const
        {
            return initialCounts_;
        }

    private:

        std::vector<ID3D11UnorderedAccessView*> unorderedViews_;
        std::vector<ID3D11ShaderResourceView*>  resourceViews_;
        std::vector<UINT>                       initialCounts_;

};


} // /namespace LLGL


#endif



// ================================================================================
