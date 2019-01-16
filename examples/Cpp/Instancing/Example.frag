// GLSL fragment shader
#version 140

layout(std140) uniform Settings
{
    mat4 vpMatrix;
    vec4 viewPos;
    vec4 fogColorAndDensity;
    vec2 animVec;
};

uniform sampler2DArray tex;

// Fragment input from the vertex shader
in vec4 vWorldPos;
in vec3 vTexCoord;
in vec3 vColor;

// Fragment output color
out vec4 fragColor;

// Fragment shader main function
void main()
{
	// Sample albedo texture
	vec4 color = texture(tex, vTexCoord);

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
