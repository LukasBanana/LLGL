/*
 * ShadowMapping.VShadow.330core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

layout(std140) uniform Scene
{
    mat4 vpMatrix;
    mat4 wMatrix;
    mat4 vpShadowMatrix;
    vec4 solidColor;
    vec4 lightVec;
};

in vec3 position;

void main()
{
	gl_Position = vpShadowMatrix * (wMatrix * vec4(position, 1));
}

