#version 450

layout(location = 0) out vec4 outOcclusion;

in vec2 vUV;

layout(binding = 0) uniform sampler2D 	uNoise;
layout(binding = 1) uniform sampler2DMS uPosition;
layout(binding = 2) uniform sampler2DMS uNormal;

layout(std140, binding = 4) uniform shader_data{
	mat4 transform;
	mat4 world;
	mat4 view;
	mat4 proj;
	mat4 viewProj;

	float time;
	vec2 screenSize;
	vec2 noiseScale;
	float near;
	float far;
	float msaaSampleCount;

	vec4 cameraPosition;

	float radius;
	float bias;
	vec4 ssaoSamples[64];
} ssArgs;

const int numSamples = 4;
const int kernelSize = 64;

void main() {
	ivec2 pos 		= ivec2(gl_FragCoord.xy);
	vec3 randomVec	= texture(uNoise, vUV * ssArgs.noiseScale).xyz;

	float occlusion = 0.0f;
	for(int msaaSampleNr = 0;
			msaaSampleNr < ssArgs.msaaSampleCount;
			msaaSampleNr++) {
		vec4 position 	= texelFetch(uPosition, pos, msaaSampleNr);
		vec4 normal 	= texelFetch(uNormal, pos, msaaSampleNr);

		if(normal == vec4(0))
		{
			outOcclusion = vec4(1);
			return;
		}

		float positionDistance = distance(ssArgs.cameraPosition.xyz, position.xyz);

		vec3 tangent	= normalize(randomVec - normal.xyz * dot(randomVec, normal.xyz));
		vec3 bitangent	= cross(normal.xyz, tangent);
		mat3 TBN		= mat3(tangent, bitangent, normal.xyz);

		for(int kernelNr = 0;
				kernelNr < kernelSize;
				++kernelNr) {
			// From tangent to world-space
			vec3 ssaoSample		= TBN * ssArgs.ssaoSamples[kernelNr].xyz;
			ssaoSample 			= position.xyz + ssaoSample * ssArgs.radius;

			vec4 offset			= vec4(ssaoSample, 1.0);
			offset				= ssArgs.viewProj * offset;	// from world to clip-space
			offset.xyz			/= offset.w;               	// perspective divide
			offset.xyz			= offset.xyz * 0.5 + 0.5; 	// transform to range 0.0 - 1.0

			ivec2 offset_tex 	= ivec2(offset.xy * ssArgs.screenSize);
			vec3 sampleNormal 	= texelFetch(uNormal, offset_tex, msaaSampleNr).xyz;
			if(sampleNormal == vec3(0))
				continue;
				
			vec3 sampleDepth 	= texelFetch(uPosition, offset_tex, msaaSampleNr).xyz;

			float sampleDistance = distance(ssArgs.cameraPosition.xyz, sampleDepth);
			float ssaoSampleDistance = distance(ssArgs.cameraPosition.xyz, ssaoSample);

			float rangeCheck	= smoothstep(0.0, 1.0, ssArgs.radius / abs(positionDistance - sampleDistance));
			occlusion			+= (sampleDistance <= ssaoSampleDistance - ssArgs.bias ? 1.0 : 0.0) * rangeCheck;
		}

		occlusion		+= occlusion;
	}

	occlusion 		/= ssArgs.msaaSampleCount;
	occlusion		= 1.0 - (occlusion / kernelSize);
	outOcclusion 	= vec4(max(1.0, occlusion), 1.0, 1.0, 1.0) *
					 vec4(vec3(occlusion), 1.0);
}