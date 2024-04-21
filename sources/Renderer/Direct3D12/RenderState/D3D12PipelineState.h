/*
 * D3D12PipelineState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_PIPELINE_STATE_H
#define LLGL_D3D12_PIPELINE_STATE_H


#include <LLGL/PipelineState.h>
#include <LLGL/ForwardDecls.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Report.h>
#include "D3D12PipelineLayout.h"
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <memory>


namespace LLGL
{


class D3D12CommandContext;
class D3D12PipelineCache;

class D3D12PipelineState : public PipelineState
{

    public:

        void SetDebugName(const char* name) override final;
        const Report* GetReport() const override final;

    public:

        // Binds the natvie PSO to the specified command context.
        virtual void Bind(D3D12CommandContext& commandContext) = 0;

        // Returns true if this is a graphics PSO.
        inline bool IsGraphicsPSO() const
        {
            return isGraphicsPSO_;
        }

        // Returns the pipeline layout this PSO was created with.
        inline const D3D12PipelineLayout* GetPipelineLayout() const
        {
            return pipelineLayout_;
        }

        // Returns the uniform to root constant map. Index for 'uniforms' -> location of root constant 32-bit value.
        inline const std::vector<D3D12RootConstantLocation>& GetRootConstantMap() const
        {
            return rootConstantMap_;
        }

    protected:

        D3D12PipelineState(
            bool                        isGraphicsPSO,
            const PipelineLayout*       pipelineLayout,
            const ArrayView<Shader*>&   shaders,
            D3D12PipelineLayout&        defaultPipelineLayout
        );

        // Stores the native PSO and updates an optional PSO cache.
        void SetNativeAndUpdateCache(ComPtr<ID3D12PipelineState>&& native, D3D12PipelineCache* pipelineCache);

        // Writes the report with the specified message and error bit.
        void ResetReport(std::string&& text, bool hasErrors = false);

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

        // Returns the mutable report object.
        inline Report& GetMutableReport()
        {
            return report_;
        }

    private:

        const bool                              isGraphicsPSO_  = false;
        ComPtr<ID3D12PipelineState>             native_;
        ComPtr<ID3D12RootSignature>             rootSignature_;
        const D3D12PipelineLayout*              pipelineLayout_ = nullptr;
        std::vector<D3D12RootConstantLocation>  rootConstantMap_;
        Report                                  report_;

};


} // /namespace LLGL


#endif



// ================================================================================
