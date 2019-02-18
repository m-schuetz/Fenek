
#version 450

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_NV_shader_atomic_int64 : enable

layout(local_size_x = 16, local_size_y = 16) in;

struct Vertex{
	float x;
	float y;
	float z;
	uint colors;
	//float ar;
	//float ag;
	//float ab;
	//float aw;
};

layout (std430, binding=0) buffer point_data {
	Vertex vertices[];
};

layout (std430, binding=1) buffer framebuffer_data {
	//uint ssFramebuffer[];
	uint64_t ssFramebuffer[];
};

layout(rgba8ui, binding = 0) uniform uimage2D uOutput;

// void main(){
// 	uvec2 id = gl_LocalInvocationID.xy;
// 	id.x += gl_WorkGroupSize.x * gl_WorkGroupID.x;
// 	id.y += gl_WorkGroupSize.y * gl_WorkGroupID.y;

// 	ivec2 imgSize = imageSize(uOutput);

// 	ivec2 pixelCoords = ivec2(id);
// 	ivec2 sourceCoords = ivec2(id);
// 	int pixelID = sourceCoords.x + sourceCoords.y * imgSize.x;

	
// 	uint count = ssFramebuffer[pixelID];

// 	uvec4 icolor = uvec4(count, 0, 0, 255);

// 	//if(count > 100){
// 	//	icolor = uvec4(255, 0, 0, 255);
// 	//}else{
// 	//	icolor = uvec4(0, 255, 0, 255);
// 	//}


// 	imageStore(uOutput, pixelCoords, icolor);
// 	ssFramebuffer[pixelID] = 0;
// }



uvec4 colorAt(int pixelID){
	uint64_t val64 = ssFramebuffer[pixelID];
	uint ucol = uint(val64 & 0x00FFFFFFUL);
	vec4 color = 255.0 * unpackUnorm4x8(ucol);
	uvec4 icolor = uvec4(color);

	return icolor;
}

void main(){
	uvec2 id = gl_LocalInvocationID.xy;
	id.x += gl_WorkGroupSize.x * gl_WorkGroupID.x;
	id.y += gl_WorkGroupSize.y * gl_WorkGroupID.y;

	ivec2 imgSize = imageSize(uOutput);

	ivec2 pixelCoords = ivec2(id);
	ivec2 sourceCoords = ivec2(id);
	int pixelID = sourceCoords.x + sourceCoords.y * imgSize.x;

	
	uvec4 icolor = colorAt(pixelID);


	//if(val64 != 0xffffffffff000000UL){
	imageStore(uOutput, pixelCoords, icolor);
	ssFramebuffer[pixelID] = 0xffffffffff000000UL;
	//}

}

