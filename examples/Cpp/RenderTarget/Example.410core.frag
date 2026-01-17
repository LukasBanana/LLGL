// GLSL model fragment shader

#version 410

layout(std140) uniform Settings
{
	mat4 wvpMatrix;
	mat4 wMatrix;
	vec3 lightDir;
	int  useTexture2DMS;
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
	float NdotL = dot(lightDir, normalize(vNormal));
	color.rgb *= mix(0.2, 1.0, NdotL);
    
    fragColor = color;
}
