// GLSL model fragment shader

#version 450

#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

layout(std140) uniform Settings
{
	mat4 wvpMatrix;
	mat4 wMatrix;
	vec3 lightDir;
	int  useTexture2DMS;
};

uniform sampler2D colorMap;

#ifdef ENABLE_CUSTOM_MULTISAMPLING
uniform sampler2DMS colorMapMS;
#endif

in vec3 vNormal;
in vec2 vTexCoord;

out vec4 fragColor;

vec4 SampleColorMap(vec2 texCoord)
{
    #ifdef ENABLE_CUSTOM_MULTISAMPLING
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
		return texture(colorMap, texCoord);
	}
}

void main()
{
    vec4 color = SampleColorMap(vTexCoord);
    
	// Apply lambert factor for simple shading
	float NdotL = dot(lightDir, normalize(vNormal));
	color.rgb *= mix(0.2, 1.0, NdotL);
    
    fragColor = color;
}
