/*
 * ClearScreen.330core.vert
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#version 330 core

void main()
{
    int id = gl_VertexID;
	gl_Position = vec4(
        id == 1 ? 3.0 : -1.0,
        id == 2 ? -3.0 : 1.0,
        1.0,
        1.0
    );
}
