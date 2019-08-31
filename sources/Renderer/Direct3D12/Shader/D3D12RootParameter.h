/*
 * D3D12RootParameter.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_ROOT_PARAMETER_H
#define LLGL_D3D12_ROOT_PARAMETER_H


#include <d3d12.h>
#include <vector>


namespace LLGL
{


// Helper class to manage a root parameter of a root signature
class D3D12RootParameter
{

    public:

        D3D12RootParameter(D3D12_ROOT_PARAMETER* managedRootParam);

        D3D12RootParameter(const D3D12RootParameter&) = default;
        D3D12RootParameter& operator = (const D3D12RootParameter&) = default;

        void InitAsConstants(UINT shaderRegister, UINT num32BitValues, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptorCBV(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptorSRV(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptorUAV(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT shaderRegister, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        void InitAsDescriptorTable(UINT maxNumDescriptorRanges, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

        void AppendDescriptorTableRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT baseShaderRegister, UINT numDescriptors = 1, UINT registerSpace = 0);

        void Clear();

        // Returns true if the specified descriptor range type is compatible with this root paramter.
        bool IsCompatible(D3D12_DESCRIPTOR_RANGE_TYPE rangeType) const;

    private:

        D3D12_ROOT_PARAMETER*               managedRootParam_   = nullptr;
        std::vector<D3D12_DESCRIPTOR_RANGE> descRanges_;

};


} // /namespace LLGL


#endif



// ================================================================================
