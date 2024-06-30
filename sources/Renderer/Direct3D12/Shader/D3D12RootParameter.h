/*
 * D3D12RootParameter.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_ROOT_PARAMETER_H
#define LLGL_D3D12_ROOT_PARAMETER_H


#include <LLGL/Container/SmallVector.h>
#include <string>
#include <d3d12.h>


namespace LLGL
{


struct BindingSlot;

// Helper class to manage a root parameter of a root signature
class D3D12RootParameter
{

    public:

        D3D12RootParameter() = default;
        D3D12RootParameter(D3D12_ROOT_PARAMETER* managedRootParam);

        D3D12RootParameter(const D3D12RootParameter&) = default;
        D3D12RootParameter& operator = (const D3D12RootParameter&) = default;

        void InitAsConstants(const D3D12_ROOT_CONSTANTS& rootConstants, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsConstants(UINT shaderRegister, UINT num32BitValues, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE paramType, const BindingSlot& slot, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptorCBV(const BindingSlot& slot, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptorSRV(const BindingSlot& slot, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptorUAV(const BindingSlot& slot, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT shaderRegister, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptorTable(UINT maxNumDescriptorRanges, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

        void AppendDescriptorTableRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT baseShaderRegister, UINT numDescriptors = 1, UINT registerSpace = 0);
        void AppendDescriptorTableRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, const BindingSlot& slot, UINT numDescriptors = 1);

        void IncludeShaderVisibility(D3D12_SHADER_VISIBILITY visibility);

        void Clear();

        // Returns true if the specified descriptor range type is compatible with this root paramter.
        bool IsCompatible(D3D12_ROOT_PARAMETER_TYPE rootParamType, D3D12_DESCRIPTOR_RANGE_TYPE rangeType) const;

        // Returns true if the specified root constants are compatible with this root paramter.
        bool IsCompatible(const D3D12_ROOT_CONSTANTS& rootConstants, D3D12_SHADER_VISIBILITY visibility) const;

    public:

        // Returns the best suitable shader visibility for the specified stage flags.
        static D3D12_SHADER_VISIBILITY FindSuitableVisibility(long stageFlags);

    private:

        D3D12_ROOT_PARAMETER*                   managedRootParam_   = nullptr;
        SmallVector<D3D12_DESCRIPTOR_RANGE, 8>  descRanges_;

};


} // /namespace LLGL


#endif



// ================================================================================
