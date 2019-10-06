/*
 * D3D12PipelineState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_PIPELINE_STATE_H
#define LLGL_D3D12_PIPELINE_STATE_H


#include <LLGL/PipelineState.h>
#include <LLGL/ForwardDecls.h>
#include "../../DXCommon/ComPtr.h"
#include "../../Serialization.h"
#include <d3d12.h>
#include <memory>


namespace LLGL
{


class D3D12CommandContext;
class D3D12PipelineLayout;

class D3D12PipelineState : public PipelineState
{

    public:

        void SetName(const char* name) override final;

    public:

        // Binds the natvie PSO to the specified command context.
        virtual void Bind(D3D12CommandContext& commandContext) = 0;

        // Returns true if this is a graphics PSO.
        inline bool IsGraphicsPSO() const
        {
            return isGraphicsPSO_;
        }

    protected:

        D3D12PipelineState(
            bool                    isGraphicsPSO,
            const PipelineLayout*   pipelineLayout,
            D3D12PipelineLayout&    defaultPipelineLayout
        );

        D3D12PipelineState(
            bool                            isGraphicsPSO,
            ID3D12Device*                   device,
            Serialization::Deserializer&    reader
        );

        // Stores the native PSO.
        void SetNative(ComPtr<ID3D12PipelineState>&& native);

        // Returns the native PSO object.
        inline ID3D12PipelineState* GetNative() const
        {
            return native_.Get();
        }

        // Returns the root signature this PSO was linked to.
        inline ID3D12RootSignature* GetRootSignature() const
        {
            return rootSignature_.Get();
        }

    private:

        const bool                  isGraphicsPSO_  = false;
        ComPtr<ID3D12PipelineState> native_;
        ComPtr<ID3D12RootSignature> rootSignature_;

};


} // /namespace LLGL


#endif



// ================================================================================
