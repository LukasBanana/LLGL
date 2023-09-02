// GLSL model fragment shader

#version 140

#ifdef GL_ES
precision mediump float;
#endif

in vec2 vTexCoord;
in vec4 vColor;

out vec4 fragColor;

uniform sampler2D glyphTexture;

void main()
{
    fragColor = vec4(vColor.rgb, vColor.a * texture(glyphTexture, vTexCoord).a);
}
