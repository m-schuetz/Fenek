

#version 450
//layout(local_size_x = 8, local_size_y = 8) in;
layout(local_size_x = 16, local_size_y = 16) in;

layout(rgba8ui, binding = 0) uniform uimage2D uIndices;

layout(std430, binding = 1) buffer ssIndirectCommand{
	uint count;
	uint primCount;
	uint firstIndex;
	uint baseVertex;
	uint baseInstance;
};

layout(std430, binding = 3) buffer ssIndices{
	uint indices[];
};

void main() {
	
	uvec2 id = gl_LocalInvocationID.xy + gl_WorkGroupSize.xy * gl_WorkGroupID.xy;
	uint workGroupSize = gl_WorkGroupSize.x * gl_WorkGroupSize.y;
	uint inputIndex = gl_WorkGroupID.x * workGroupSize 
		+ gl_WorkGroupID.y * gl_NumWorkGroups.x * workGroupSize
		+ gl_LocalInvocationIndex;
	ivec2 pixelCoords = ivec2(id);
	
	uvec4 vVertexID = imageLoad(uIndices, pixelCoords);

	// check if index is not empty (kind of wrong though, also returns at gl_VertexID == 0
	if(vVertexID.r == 0 && vVertexID.g == 0 && vVertexID.b == 0){
		return;
	}

	uint vertexID = vVertexID.r | (vVertexID.g << 8) | (vVertexID.b << 16) | (vVertexID.a << 24);
		
	uint counter = atomicAdd(count, 1);
	
	//indices[counter] = 8;
	indices[counter] = vertexID;
	
}






