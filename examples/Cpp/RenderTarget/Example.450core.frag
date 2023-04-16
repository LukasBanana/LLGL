// GLSL model fragment shader

#version 450

layout(std140, binding = 3) uniform Settings
{
	mat4 wvpMatrix;
	mat4 wMatrix;
	int useTexture2DMS;
};

layout(binding = 1) uniform sampler colorMapSampler;
layout(binding = 2) uniform texture2D colorMap;
//layout(binding = 3) uniform sampler2DMS colorMapMS;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec4 fragColor;

vec4 SampleColorMap(vec2 texCoord)
{
    #if 0
	if (useTexture2DMS != 0)
	{
		// Fetch texel from multi-sample texture
		ivec2 size = textureSize(colorMapMS);
		int numSamples = textureSamples(colorMapMS);
		
		ivec2 tc = ivec2(
			int(texCoord.x * float(size.x)),
			int(texCoord.y * float(size.y))
		);
		
		// Compute average of all samples
		vec4 c = vec4(0);
		
		for (int i = 0; i < numSamples; ++i)
			c += texelFetch(colorMapMS, tc, i);
		
		c /= numSamples;
		
		return c;
	}
	else
    #endif
	{
		// Sample texel from standard texture
		return texture(sampler2D(colorMap, colorMapSampler), texCoord);
	}
}

void main()
{
    vec4 color = SampleColorMap(vTexCoord);
    
	// Apply lambert factor for simple shading
	const vec3 lightVec = vec3(0, 0, -1);
	float NdotL = dot(lightVec, normalize(vNormal));
	color.rgb *= mix(0.2, 1.0, NdotL);
    
    fragColor = color;
}
