#version 140

layout(std140) uniform Settings
{
    layout(row_major) mat4 vpMatrix;
    vec4 viewPos;
    vec3 fogColor;
    float fogDensity;
    vec2 animVec;
};

uniform sampler2DArray s_textexSampler;

in vec4 v_WORLDPOS;
in vec3 v_TEXCOORD;
in vec3 v_COLOR;
out vec4 SV_Target;

void main()
{
    vec4 _45 = texture(s_textexSampler, v_TEXCOORD);
    if ((_45.w - 0.5) < 0.0)
    {
        discard;
    }
    float _56 = distance(viewPos, v_WORLDPOS) * fogDensity;
    vec3 _66 = mix(_45.xyz * v_COLOR, fogColor, vec3(1.0 - (1.0 / exp(_56 * _56))));
    SV_Target = vec4(_66.x, _66.y, _66.z, _45.w);
}

