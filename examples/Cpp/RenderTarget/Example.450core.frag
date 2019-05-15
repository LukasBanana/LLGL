// GLSL model fragment shader

#version 450

layout(std140, binding = 3) uniform Settings
{
	mat4 wvpMatrix;
	int useTexture2DMS;
};

layout(binding = 1) uniform sampler colorMapSampler;
layout(binding = 2) uniform texture2D colorMap;
//layout(binding = 3) uniform sampler2DMS colorMapMS;

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 fragColor;

void main()
{
    #if 0
	if (useTexture2DMS != 0)
	{
		// Fetch texel from multi-sample texture
		ivec2 size = textureSize(colorMapMS);
		int numSamples = textureSamples(colorMapMS);
		
		ivec2 tc = ivec2(
			int(vTexCoord.x * float(size.x)),
			int(vTexCoord.y * float(size.y))
		);
		
		// Compute average of all samples
		vec4 c = vec4(0);
		
		for (int i = 0; i < numSamples; ++i)
			c += texelFetch(colorMapMS, tc, i);
		
		c /= numSamples;
		
		fragColor = c;
	}
	else
    #endif
	{
		// Sample texel from standard texture
		fragColor = texture(sampler2D(colorMap, colorMapSampler), vTexCoord);
	}
}
