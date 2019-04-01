#version 450

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2DMS uColor;
layout(binding = 1) uniform sampler2DMS uOcclusion;

const int numSamples = 4;

void main() {
	ivec2 pos 		= ivec2(gl_FragCoord.xy);
	vec4 f_color	= vec4(0,0,0,1);

	for(int msaaSampleNr = 0;
			msaaSampleNr < numSamples;
			msaaSampleNr++) {
		vec4 color 		= texelFetch(uColor, pos, msaaSampleNr);
		vec4 occlusion	= texelFetch(uOcclusion, pos, msaaSampleNr);

		f_color += color * occlusion;
		//f_color += max(vec4(2),color)  * occlusion;
	}

	f_color /= numSamples;
	outColor = f_color;
}