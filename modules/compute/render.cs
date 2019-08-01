
#version 450

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_NV_shader_atomic_int64 : enable

layout(local_size_x = 128, local_size_y = 1) in;

struct Vertex{
	float x;
	float y;
	float z;
	uint colors;
};

layout(location = 0) uniform mat4 uTransform;

layout (std430, binding = 0) buffer point_data {
	Vertex vertices[];
};

layout (std430, binding = 1) buffer framebuffer_data {
	//uint ssFramebuffer[];
	//int64_t ssFramebuffer[];
	uint64_t ssFramebuffer[];
};

uniform ivec2 uImageSize;

layout(binding = 0) uniform sampler2D uGradient;


void main(){

	uint globalID = gl_GlobalInvocationID.x;
	//globalID += 1000000;

	Vertex v = vertices[globalID];

	vec4 pos = uTransform * vec4(v.x, v.y, v.z, 1.0);
	pos.xyz = pos.xyz / pos.w;

	if(pos.w <= 0.0 || pos.x < -1.0 || pos.x > 1.0 || pos.y < -1.0 || pos.y > 1.0){
		return;
	}

	if(v.x < 130 || v.x > 165){
		return;
	}
	if(v.y < 151 || v.y > 180){
		return;
	}

	vec2 imgPos = (pos.xy * 0.5 + 0.5) * uImageSize;
	ivec2 pixelCoords = ivec2(imgPos);
	int pixelID = pixelCoords.x + pixelCoords.y * uImageSize.x;

	double depth = pos.w;
	uint64_t u64Depth = uint64_t(depth * 1000000.0);

	uint64_t u64Colors = v.colors;
	uint64_t val64 = (u64Depth << 24) | v.colors;


	atomicMin(ssFramebuffer[pixelID], val64);
	//atomicMin(ssFramebuffer[pixelID + 1], val64);
	//atomicMin(ssFramebuffer[pixelID + uImageSize.x], val64);
	//atomicMin(ssFramebuffer[pixelID + uImageSize.x + 1], val64);
	
	
	//atomicAdd(ssFramebuffer[pixelID], 1l);

}

