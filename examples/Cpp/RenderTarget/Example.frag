// GLSL model fragment shader

#version 140

layout(std140) uniform Settings
{
	mat4 wvpMatrix;
	int useTexture2DMS;
};

uniform sampler2D colorMap;
//uniform sampler2DMS colorMapMS;

in vec2 vTexCoord;

out vec4 fragColor;

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
		fragColor = texture(colorMap, vTexCoord);
	}
}
