#version 120

varying vec3 v_COLOR;

void main()
{
    gl_FragData[0] = vec4(v_COLOR, 1.0);
}

