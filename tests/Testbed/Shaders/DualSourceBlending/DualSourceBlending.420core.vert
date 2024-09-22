/*
 * DualSourceBlending.420core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 420 core

layout(location = 0) out vec2 vTexCoord;

void main()
{
    uint id = gl_VertexID;
	gl_Position = vec4(id == 2 ? 3.0 : -1.0, id == 0 ? 3.0 : -1.0, 1.0, 1.0);
	vTexCoord = gl_Position.xy * vec2(0.5, -0.5) + 0.5;
}
