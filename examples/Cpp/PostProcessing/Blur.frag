// GLSL blur fragment shader

#version 140

layout(std140) uniform BlurSettings
{
	vec2 blurShift;
};

uniform sampler2D glossMap;

in vec2 vTexCoord;

out vec4 fragColor;

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
	
	c += texture(glossMap, vTexCoord - blurShift*5) * GAUSSIAN_KERNEL_1;
	c += texture(glossMap, vTexCoord - blurShift*4) * GAUSSIAN_KERNEL_2;
	c += texture(glossMap, vTexCoord - blurShift*3) * GAUSSIAN_KERNEL_3;
	c += texture(glossMap, vTexCoord - blurShift*2) * GAUSSIAN_KERNEL_4;
	c += texture(glossMap, vTexCoord - blurShift  ) * GAUSSIAN_KERNEL_5;
	c += texture(glossMap, vTexCoord              ) * GAUSSIAN_KERNEL_6;
	c += texture(glossMap, vTexCoord + blurShift  ) * GAUSSIAN_KERNEL_5;
	c += texture(glossMap, vTexCoord + blurShift*2) * GAUSSIAN_KERNEL_4;
	c += texture(glossMap, vTexCoord + blurShift*3) * GAUSSIAN_KERNEL_3;
	c += texture(glossMap, vTexCoord + blurShift*4) * GAUSSIAN_KERNEL_2;
	c += texture(glossMap, vTexCoord + blurShift*5) * GAUSSIAN_KERNEL_1;
	
	fragColor = c;
}
