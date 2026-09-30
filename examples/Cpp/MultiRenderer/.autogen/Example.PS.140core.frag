#version 140

uniform sampler2D s_colorMapcolorMapSampler;

in vec3 v_NORMAL;
in vec2 v_TEXCOORD;
out vec4 SV_Target;

void main()
{
    vec4 _34 = texture(s_colorMapcolorMapSampler, v_TEXCOORD);
    vec4 _37 = mix(vec4(1.0), _34, vec4(_34.w));
    vec3 _42 = _37.xyz * mix(0.20000000298023223876953125, 1.0, dot(vec3(0.0, 0.0, -1.0), normalize(v_NORMAL)));
    SV_Target = vec4(_42.x, _42.y, _42.z, _37.w);
}

