// GLSL model fragment shader

#version 410

layout(std140) uniform Settings
{
	mat4 wvpMatrix;
	mat4 wMatrix;
	int useTexture2DMS;
};

uniform sampler2D colorMap;

in vec3 vNormal;
in vec2 vTexCoord;

out vec4 fragColor;

void main()
{
    // Sample texel from standard texture
    vec4 color = texture(colorMap, vTexCoord);
    
	// Apply lambert factor for simple shading
	const vec3 lightVec = vec3(0, 0, -1);
	float NdotL = dot(lightVec, normalize(vNormal));
	color.rgb *= mix(0.2, 1.0, NdotL);
    
    fragColor = color;
}
