// GLSL blur fragment shader

#version 450 core

layout(std140, binding = 2) uniform BlurSettings
{
	vec2 blurShift;
};

layout(binding = 4) uniform texture2D glossMap;
layout(binding = 6) uniform sampler glossMapSampler;

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 fragColor;

// Static values for the 1-dimensional gaussian kernel (sigma = 2.2, size = 11)
// see http://dev.theomader.com/gaussian-kernel-calculator/
#define GAUSSIAN_KERNEL_1 0.014374
#define GAUSSIAN_KERNEL_2 0.035855
#define GAUSSIAN_KERNEL_3 0.072994
#define GAUSSIAN_KERNEL_4 0.121281
#define GAUSSIAN_KERNEL_5 0.164472
#define GAUSSIAN_KERNEL_6 0.182049

void main()
{
	// Accumulate the samples with the gaussian kernel
	vec4 c = vec4(0);
	
	c += texture(sampler2D(glossMap, glossMapSampler), vTexCoord - blurShift*5) * GAUSSIAN_KERNEL_1;
	c += texture(sampler2D(glossMap, glossMapSampler), vTexCoord - blurShift*4) * GAUSSIAN_KERNEL_2;
	c += texture(sampler2D(glossMap, glossMapSampler), vTexCoord - blurShift*3) * GAUSSIAN_KERNEL_3;
	c += texture(sampler2D(glossMap, glossMapSampler), vTexCoord - blurShift*2) * GAUSSIAN_KERNEL_4;
	c += texture(sampler2D(glossMap, glossMapSampler), vTexCoord - blurShift  ) * GAUSSIAN_KERNEL_5;
	c += texture(sampler2D(glossMap, glossMapSampler), vTexCoord              ) * GAUSSIAN_KERNEL_6;
	c += texture(sampler2D(glossMap, glossMapSampler), vTexCoord + blurShift  ) * GAUSSIAN_KERNEL_5;
	c += texture(sampler2D(glossMap, glossMapSampler), vTexCoord + blurShift*2) * GAUSSIAN_KERNEL_4;
	c += texture(sampler2D(glossMap, glossMapSampler), vTexCoord + blurShift*3) * GAUSSIAN_KERNEL_3;
	c += texture(sampler2D(glossMap, glossMapSampler), vTexCoord + blurShift*4) * GAUSSIAN_KERNEL_2;
	c += texture(sampler2D(glossMap, glossMapSampler), vTexCoord + blurShift*5) * GAUSSIAN_KERNEL_1;
	
	fragColor = c;
}
