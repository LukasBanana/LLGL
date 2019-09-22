/*
 * D3D12QueryHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_QUERY_HEAP_H
#define LLGL_D3D12_QUERY_HEAP_H


#include <LLGL/QueryHeap.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12Device;

class D3D12QueryHeap final : public QueryHeap
{

    public:

        void SetName(const char* name) override;

    public:

        D3D12QueryHeap(D3D12Device& device, const QueryHeapDescriptor& desc);

        void Begin(ID3D12GraphicsCommandList* commandList, UINT query);
        void End(ID3D12GraphicsCommandList* commandList, UINT query);

        // Resolves all queries if not already done.
        void FlushDirtyRange(ID3D12GraphicsCommandList* commandList);

        // Returns true if this query heap has a dirty range that must be resolved before the query data can be retrieved.
        bool HasDirtyRange() const;

        // Returns true if the specified range of queries overlaps with the dirty range.
        bool InsideDirtyRange(UINT firstQuery, UINT numQueries) const;

        // Maps the query result buffer to CPU local memory.
        void* Map(UINT firstQuery, UINT numQueries);
        void Unmap();

        // Returns the aligend buffer offset within the result resource for the specified query.
        UINT64 GetAlignedBufferOffest(UINT query) const;

        // Returns the native D3D12_QUERY_TYPE type.
        inline D3D12_QUERY_TYPE GetNativeType() const
        {
            return nativeType_;
        }

        // Returns the native ID3D12QueryHeap object.
        inline ID3D12QueryHeap* GetNative() const
        {
            return native_.Get();
        }

        // Returns the result buffer resource object.
        inline ID3D12Resource* GetResultResource() const
        {
            return resultResource_.Get();
        }

        // Returns true if this query heap is used as predicate for conditional rendering.
        inline bool IsPredicate() const
        {
            return isPredicate_;
        }

    private:

        void InvalidateDirtyRange();
        void MarkDirtyRange(UINT firstQuery, UINT numQueries);

        void ResolveData(ID3D12GraphicsCommandList* commandList, UINT firstQuery, UINT numQueries);

        void TransitionResource(
            ID3D12GraphicsCommandList*  commandList,
            D3D12_RESOURCE_STATES       stateBefore,
            D3D12_RESOURCE_STATES       stateAfter
        );

        void CopyResultsToResource(
            ID3D12GraphicsCommandList*  commandList,
            UINT                        firstQuery,
            UINT                        numQueries
        );

    private:

        D3D12_QUERY_TYPE        nativeType_     = D3D12_QUERY_TYPE_OCCLUSION;
        ComPtr<ID3D12QueryHeap> native_;
        ComPtr<ID3D12Resource>  resultResource_;
        UINT                    alignedStride_  = 0;
        UINT                    queryPerType_   = 1;
        bool                    isPredicate_    = false;
        UINT                    dirtyRange_[2]  = {};       // Begin/end range of queries that need to be resolved

};


} // /namespace LLGL


#endif



// ================================================================================
