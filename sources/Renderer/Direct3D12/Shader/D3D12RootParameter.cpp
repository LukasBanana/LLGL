/*
 * D3D12RootParameter.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RootParameter.h"


namespace LLGL
{


D3D12RootParameter::D3D12RootParameter(D3D12_ROOT_PARAMETER* managedRootParam) :
    managedRootParam_ { managedRootParam }
{
}

void D3D12RootParameter::InitAsConstants(UINT shaderRegister, UINT num32BitValues, D3D12_SHADER_VISIBILITY visibility)
{
    managedRootParam_->ParameterType                = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    managedRootParam_->Constants.ShaderRegister     = shaderRegister;
    managedRootParam_->Constants.RegisterSpace      = 0;
    managedRootParam_->Constants.Num32BitValues     = num32BitValues;
    managedRootParam_->ShaderVisibility             = visibility;
}

void D3D12RootParameter::InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility)
{
    managedRootParam_->ParameterType                = paramType;
    managedRootParam_->Descriptor.ShaderRegister    = shaderRegister;
    managedRootParam_->Descriptor.RegisterSpace     = 0;
    managedRootParam_->ShaderVisibility             = visibility;
}

void D3D12RootParameter::InitAsDescriptorCBV(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility)
{
    InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, shaderRegister, visibility);
}

void D3D12RootParameter::InitAsDescriptorSRV(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility)
{
    InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, shaderRegister, visibility);
}

void D3D12RootParameter::InitAsDescriptorUAV(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility)
{
    InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, shaderRegister, visibility);
}

void D3D12RootParameter::InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT shaderRegister, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility)
{
    InitAsDescriptorTable(1, visibility);
    AppendDescriptorTableRange(rangeType, shaderRegister, numDescriptors);
}

void D3D12RootParameter::InitAsDescriptorTable(UINT maxNumDescriptorRanges, D3D12_SHADER_VISIBILITY visibility)
{
    descRanges_.reserve(maxNumDescriptorRanges);
    managedRootParam_->ParameterType                        = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    managedRootParam_->DescriptorTable.NumDescriptorRanges  = 0;
    managedRootParam_->DescriptorTable.pDescriptorRanges    = descRanges_.data();
    managedRootParam_->ShaderVisibility                     = visibility;
}

void D3D12RootParameter::AppendDescriptorTableRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT baseShaderRegister, UINT numDescriptors, UINT registerSpace)
{
    /* Add new descriptor range to array */
    descRanges_.resize(descRanges_.size() + 1);
    auto& descRange = descRanges_.back();

    /* Initialize descriptor range */
    descRange.RangeType                         = rangeType;
    descRange.NumDescriptors                    = numDescriptors;
    descRange.BaseShaderRegister                = baseShaderRegister;
    descRange.RegisterSpace                     = registerSpace;
    descRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    /* Increment descriptor range count */
    ++managedRootParam_->DescriptorTable.NumDescriptorRanges;
}

void D3D12RootParameter::Clear()
{
    *managedRootParam_ = {};
    descRanges_.clear();
}

static bool AreRangeTypesCompatible(D3D12_DESCRIPTOR_RANGE_TYPE lhs, D3D12_DESCRIPTOR_RANGE_TYPE rhs)
{
    /*
    Samplers are not allowed in the same descriptor table as CBVs, SRVs, and UAVs.
    see https://msdn.microsoft.com/en-us/library/windows/desktop/dn859382(v=vs.85).aspx
    */
    return
    (
        ( lhs == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER && rhs == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER ) ||
        ( lhs != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER && rhs != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER )
    );
}

bool D3D12RootParameter::IsCompatible(D3D12_DESCRIPTOR_RANGE_TYPE rangeType) const
{
    if (descRanges_.empty())
        return true;
    else
        return AreRangeTypesCompatible(descRanges_.back().RangeType, rangeType);
}


} // /namespace LLGL



// ================================================================================
