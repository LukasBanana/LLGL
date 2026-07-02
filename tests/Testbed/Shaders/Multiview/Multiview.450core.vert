/*
 * Multiview.450core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 *
 * Testbed multiview (single-pass layered) vertex shader for VK_KHR_multiview. Renders a full-screen triangle
 * whose color depends on gl_ViewIndex, so each view writes a distinct color into its own array layer.
 */

#version 450 core

#extension GL_EXT_multiview : require

layout(location = 0) out vec3 vColor;

void main()
{
    vec2 pos = vec2(
        gl_VertexIndex == 1 ?  3.0 : -1.0,
        gl_VertexIndex == 2 ? -3.0 :  1.0
    );
    gl_Position = vec4(pos, 1.0, 1.0);

    // View 0 -> red, view 1 -> green: distinct per-view colors to verify view -> layer routing.
    vColor = (gl_ViewIndex == 0 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0));
}
