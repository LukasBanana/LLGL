/*
 * SystemValue.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SYSTEM_VALUE_H
#define LLGL_SYSTEM_VALUE_H


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Enumeration of system values for vertex input or fragment output attributes.
\remarks This is only used for shader code reflection.
\remarks Here is an overview of all supported system values and their mapping to the respective shading language:
<table>
<caption>System value mapping</caption>
<tr><th>Type</th><th>HLSL</th><th>GLSL</th><th>SPIR-V</th><th>Metal</th></tr>
<tr><td>SystemValue::ClipDistance</td><td>\c SV_ClipDistance</td><td>\c gl_ClipDistance</td><td>\c gl_ClipDistance</td><td><code>[[clip_distance]]</code></td></tr>
<tr><td>SystemValue::Color</td><td>\c SV_Target</td><td>N/A</td><td>N/A</td><td><code>[[color(<i>N</i>)]]</code></td></tr>
<tr><td>SystemValue::CullDistance</td><td>\c SV_CullDistance</td><td>\c gl_CullDistance</td><td>\c gl_CullDistance</td><td>N/A</td></tr>
<tr><td>SystemValue::Depth</td><td>\c SV_Depth</td><td>\c gl_FragDepth</td><td>\c gl_FragDepth</td><td><code>[[depth(any)]]</code></td></tr>
<tr><td>SystemValue::DepthGreater</td><td>\c SV_DepthGreaterEqual</td><td>N/A</td><td>N/A</td><td><code>[[depth(greater)]]</code></td></tr>
<tr><td>SystemValue::DepthLess</td><td>\c SV_DepthLessEqual</td><td>N/A</td><td>N/A</td><td><code>[[depth(less)]]</code></td></tr>
<tr><td>SystemValue::FrontFacing</td><td>\c SV_IsFrontFace</td><td>\c gl_FrontFacing</td><td>\c gl_FrontFacing</td><td><code>[[front_facing]]</code></td></tr>
<tr><td>SystemValue::InstanceID</td><td>\c SV_InstanceID</td><td>\c gl_InstanceID</td><td>\c gl_InstanceIndex</td><td><code>[[instance_id]</code></td></tr>
<tr><td>SystemValue::Position</td><td>\c SV_Position</td><td>\c gl_Position, \c gl_FragCoord</td><td>\c gl_Position, \c gl_FragCoord</td><td><code>[[position]]</code></td></tr>
<tr><td>SystemValue::PrimitiveID</td><td>\c SV_PrimitiveID</td><td>\c gl_PrimitiveID</td><td>\c gl_PrimitiveID</td><td>N/A</td></tr>
<tr><td>SystemValue::RenderTargetIndex</td><td>\c SV_RenderTargetArrayIndex</td><td>\c gl_Layer</td><td>\c gl_Layer</td><td><code>[[render_target_array_index]]</code></td></tr>
<tr><td>SystemValue::SampleMask</td><td>\c SV_Coverage</td><td>\c gl_SampleMask</td><td>\c gl_SampleMask</td><td><code>[[sample_mask]]</code></td></tr>
<tr><td>SystemValue::SampleID</td><td>\c SV_SampleIndex</td><td>\c gl_SampleID</td><td>\c gl_SampleID</td><td><code>[[sample_id]]</code></td></tr>
<tr><td>SystemValue::Stencil</td><td>\c SV_StencilRef</td><td>N/A</td><td>N/A</td><td><code>[[stencil]]</code></td></tr>
<tr><td>SystemValue::VertexID</td><td>\c SV_VertexID</td><td>\c gl_VertexID</td><td>\c gl_VertexIndex</td><td><code>[[vertex_id]]</code></td></tr>
<tr><td>SystemValue::ViewportIndex</td><td>\c SV_ViewportArrayIndex</td><td>\c gl_ViewportIndex</td><td>\c gl_ViewportIndex</td><td><code>[[viewport_array_index]]</code></td></tr>
</table>
\see VertexAttribute::systemValue
\see FragmentAttribute::systemValue
*/
enum class SystemValue
{
    //! Undefined system value.
    Undefined,

    //! Forward-compatible mechanism for vertex clipping.
    ClipDistance,

    //! Fragment output color value.
    Color,

    //! Mechanism for controlling user culling.
    CullDistance,

    //! Fragment depth value.
    Depth,

    //! Fragment depth value that is greater than or equal to the previous one.
    DepthGreater,

    //! Fragment depth value that is less than or equal to the previous one.
    DepthLess,

    //! Indicates whether a primitive is front or back facing.
    FrontFacing,

    /**
    \brief Index of the input instance.
    \note This value behalves differently between Direct3D and OpenGL.
    \see CommandBuffer::DrawInstanced(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)
    */
    InstanceID,

    //! Vertex or fragment position.
    Position,

    //! Index of the geometry primitive.
    PrimitiveID,

    //! Index of the render target layer.
    RenderTargetIndex,

    //! Sample coverage mask.
    SampleMask,

    //! Index of the input sample.
    SampleID,

    /**
    \brief Fragment stencil value.
    \note Only supported with: Direct3D 11.3, Direct3D 12, Metal.
    */
    Stencil,

    /**
    \brief Index of the input vertex.
    \note This value behalves differently between Direct3D and OpenGL.
    \see CommandBuffer::Draw
    */
    VertexID,

    //! Index of the viewport array.
    ViewportIndex,
};


} // /namespace LLGL


#endif



// ================================================================================
