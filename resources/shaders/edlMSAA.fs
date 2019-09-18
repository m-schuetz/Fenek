#version 450

layout(location = 0) out vec4 out_color;

in vec2 vUV;

layout(binding = 0) uniform sampler2DMS uColor;
layout(binding = 1) uniform sampler2DMS uDepth;

layout(std140, binding = 4) uniform shader_data{
	mat4 transform;
	mat4 world;
	mat4 view;
	mat4 proj;

	float time;
	vec2 screenSize;
	float near;
	float far;
	float edlStrength;
	float msaaSampleCount;

} ssArgs;

const int numSamples = 4;

ivec2 sampleLocations[4] = ivec2[](
	ivec2( 1,  0),
	ivec2( 0,  1),
	ivec2(-1,  0),
	ivec2( 0, -1)
);


// http://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
float linearize(float depth){
	// that doesn't seem to be right...
	//return 1 / pow(depth, 5.5);
	return 1 / depth;
}


float response(ivec2 pos, int msaaSampleNr){

	float d = texelFetch(uDepth, pos, msaaSampleNr).r;
	float depth = log2(linearize(d));

	float sum = 0.0;
	
	for(int i = 0; i < numSamples; i++){
		ivec2 samplePos = pos + sampleLocations[i] * 1;
		float neighborDepth = texelFetch(uDepth, samplePos, msaaSampleNr).r;
		neighborDepth = log2(linearize(neighborDepth));
		sum += max(0.0, depth - neighborDepth);
	}
	
	return sum / float(numSamples);
}

void main() {
	ivec2 pos = ivec2(gl_FragCoord.xy);
	
	vec4 col = vec4(0, 0, 0, 0);
	float sumShade = 0.0;
	for(int msaaSampleNr = 0; msaaSampleNr < ssArgs.msaaSampleCount; msaaSampleNr++){
		float res = response(pos, msaaSampleNr);
		float shade = exp(-res * 300.0 * ssArgs.edlStrength * 0.8);

		sumShade += shade;
		col += texelFetch(uColor, pos, msaaSampleNr);
	}
	float avgShade = sumShade / ssArgs.msaaSampleCount;
	col = col / ssArgs.msaaSampleCount;

	out_color = col * avgShade;
}