/*
 * D3D12RootParameter.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12RootParameter.h"
#include <LLGL/ShaderFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


D3D12RootParameter::D3D12RootParameter(D3D12_ROOT_PARAMETER* managedRootParam) :
    managedRootParam_ { managedRootParam }
{
}

void D3D12RootParameter::InitAsConstants(const D3D12_ROOT_CONSTANTS& rootConstants, D3D12_SHADER_VISIBILITY visibility)
{
    managedRootParam_->ParameterType    = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    managedRootParam_->Constants        = rootConstants;
    managedRootParam_->ShaderVisibility = visibility;
}

void D3D12RootParameter::InitAsConstants(UINT shaderRegister, UINT num32BitValues, D3D12_SHADER_VISIBILITY visibility)
{
    managedRootParam_->ParameterType                = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    managedRootParam_->Constants.ShaderRegister     = shaderRegister;
    managedRootParam_->Constants.RegisterSpace      = 0;
    managedRootParam_->Constants.Num32BitValues     = num32BitValues;
    managedRootParam_->ShaderVisibility             = visibility;
}

void D3D12RootParameter::InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility)
{
    managedRootParam_->ParameterType                = paramType;
    managedRootParam_->Descriptor.ShaderRegister    = shaderRegister;
    managedRootParam_->Descriptor.RegisterSpace     = registerSpace;
    managedRootParam_->ShaderVisibility             = visibility;
}

void D3D12RootParameter::InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE paramType, const BindingSlot& slot, D3D12_SHADER_VISIBILITY visibility)
{
    InitAsDescriptor(paramType, slot.index, slot.set, visibility);
}

void D3D12RootParameter::InitAsDescriptorCBV(const BindingSlot& slot, D3D12_SHADER_VISIBILITY visibility)
{
    InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, slot, visibility);
}

void D3D12RootParameter::InitAsDescriptorSRV(const BindingSlot& slot, D3D12_SHADER_VISIBILITY visibility)
{
    InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, slot, visibility);
}

void D3D12RootParameter::InitAsDescriptorUAV(const BindingSlot& slot, D3D12_SHADER_VISIBILITY visibility)
{
    InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, slot, visibility);
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

static bool IsIncludedInDescriptorRange(const D3D12_DESCRIPTOR_RANGE& descRange, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT baseShaderRegister, UINT registerSpace)
{
    return
    (
        rangeType          == descRange.RangeType &&
        registerSpace      == descRange.RegisterSpace &&
        baseShaderRegister >= descRange.BaseShaderRegister &&
        baseShaderRegister <  descRange.BaseShaderRegister + descRange.NumDescriptors
    );
}

void D3D12RootParameter::AppendDescriptorTableRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT baseShaderRegister, UINT numDescriptors, UINT registerSpace)
{
    /* Ignore this call if the input is already included in the current range */
    if (!descRanges_.empty() && IsIncludedInDescriptorRange(descRanges_.back(), rangeType, baseShaderRegister, registerSpace))
        return;

    /* Add new descriptor range to array */
    D3D12_DESCRIPTOR_RANGE& descRange = AppendElementNoRealloc(descRanges_);

    /* Initialize descriptor range */
    descRange.RangeType                         = rangeType;
    descRange.NumDescriptors                    = numDescriptors;
    descRange.BaseShaderRegister                = baseShaderRegister;
    descRange.RegisterSpace                     = registerSpace;
    descRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    /* Increment descriptor range count */
    managedRootParam_->DescriptorTable.NumDescriptorRanges++;
}

void D3D12RootParameter::AppendDescriptorTableRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, const BindingSlot& slot, UINT numDescriptors)
{
    AppendDescriptorTableRange(rangeType, slot.index, numDescriptors, slot.set);
}

void D3D12RootParameter::IncludeShaderVisibility(D3D12_SHADER_VISIBILITY visibility)
{
    if (managedRootParam_->ShaderVisibility != visibility)
        managedRootParam_->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
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

bool D3D12RootParameter::IsCompatible(D3D12_ROOT_PARAMETER_TYPE rootParamType, D3D12_DESCRIPTOR_RANGE_TYPE rangeType) const
{
    if (managedRootParam_ == nullptr || managedRootParam_->ParameterType != rootParamType)
        return false;
    if (descRanges_.empty())
        return true;
    return AreRangeTypesCompatible(descRanges_.back().RangeType, rangeType);
}

bool D3D12RootParameter::IsCompatible(const D3D12_ROOT_CONSTANTS& rootConstants, D3D12_SHADER_VISIBILITY visibility) const
{
    return
    (
        managedRootParam_                           != nullptr                                      &&
        managedRootParam_->ParameterType            == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS    &&
        managedRootParam_->Constants.ShaderRegister == rootConstants.ShaderRegister                 &&
        managedRootParam_->Constants.RegisterSpace  == rootConstants.RegisterSpace                  &&
      //managedRootParam_->Constants.Num32BitValues == rootConstants.Num32BitValues                 &&
        managedRootParam_->ShaderVisibility         == visibility
    );
}

D3D12_SHADER_VISIBILITY D3D12RootParameter::FindSuitableVisibility(long stageFlags)
{
    /* Return shader visibility limited to only one stage if the input flags only contains that stage */
    switch (stageFlags)
    {
        case StageFlags::VertexStage:           return D3D12_SHADER_VISIBILITY_VERTEX;
        case StageFlags::TessControlStage:      return D3D12_SHADER_VISIBILITY_HULL;
        case StageFlags::TessEvaluationStage:   return D3D12_SHADER_VISIBILITY_DOMAIN;
        case StageFlags::GeometryStage:         return D3D12_SHADER_VISIBILITY_GEOMETRY;
        case StageFlags::FragmentStage:         return D3D12_SHADER_VISIBILITY_PIXEL;
        default:                                return D3D12_SHADER_VISIBILITY_ALL; // Visibility to all stages by default
    }
}


} // /namespace LLGL



// ================================================================================
