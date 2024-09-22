/*
 * DualSourceBlending.450core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 450 core

layout(location = 0) out vec2 vTexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
    uint id = gl_VertexIndex;
	gl_Position = vec4(id == 2 ? 3.0 : -1.0, id == 0 ? 3.0 : -1.0, 1.0, 1.0);
	vTexCoord = gl_Position.xy * vec2(0.5, -0.5) + 0.5;
}
