// GLSL fragment shader
#version 450 core

layout(std140, binding = 2) uniform Settings
{
    mat4 vpMatrix;
    vec4 viewPos;
    vec4 fogColorAndDensity;
    vec2 animVec;
};

layout(binding = 3) uniform texture2DArray tex;
layout(binding = 4) uniform sampler texSampler;

// Fragment input from the vertex shader
layout(location = 0) in vec4 vWorldPos;
layout(location = 1) in vec3 vTexCoord;
layout(location = 2) in vec3 vColor;

// Fragment output color
layout(location = 0) out vec4 fragColor;

// Fragment shader main function
void main()
{
	// Sample albedo texture
	vec4 color = texture(sampler2DArray(tex, texSampler), vTexCoord);

    // Apply alpha clipping
	if (color.a < 0.5)
		discard;
	
    // Compute fog density
    float viewDist = distance(viewPos, vWorldPos);
    float fog = viewDist*fogColorAndDensity.a;
    fog = 1.0 - 1.0/exp(fog*fog);

    // Interpolate between albedo and fog color
    color.rgb = mix(color.rgb * vColor, fogColorAndDensity.rgb, fog);

	fragColor = color;
}
