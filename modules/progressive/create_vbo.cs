#version 450

//layout(local_size_x = 8, local_size_y = 8) in;
layout(local_size_x = 16, local_size_y = 16) in;

struct VertexS{
	float ux;
	float uy;
	float uz;
	uint color;
};

struct VertexT{
	float ux;
	float uy;
	float uz;
	uint color;
	uint index;
};


layout(rgba8ui, binding = 0) uniform uimage2D uIndices;

layout(std430, binding = 1) buffer ssIndirectCommand{
	uint count;
	uint primCount;
	uint firstIndex;
	uint baseVertex;
	uint baseInstance;
};

layout(std430, binding = 2) buffer ssTarget{
	VertexT targetBuffer[];
};

layout(std430, binding = 3) buffer ssSource0{
	VertexS vbo0[];
};

layout(std430, binding = 4) buffer ssSource1{
	VertexS vbo1[];
};

layout(std430, binding = 5) buffer ssSource2{
	VertexS vbo2[];
};

layout(std430, binding = 6) buffer ssSource3{
	VertexS vbo3[];
};



void main() {
	
	uvec2 id = gl_LocalInvocationID.xy + gl_WorkGroupSize.xy * gl_WorkGroupID.xy;
	ivec2 pixelCoords = ivec2(id);
	
	uvec4 vVertexID = imageLoad(uIndices, pixelCoords);

	// check if index is not empty (kind of wrong though, also returns at gl_VertexID == 0
	if(vVertexID.r == 0 && vVertexID.g == 0 && vVertexID.b == 0){
		return;
	}

	uint vertexID = vVertexID.r | (vVertexID.g << 8) | (vVertexID.b << 16) | (vVertexID.a << 24);
	
		
	uint counter = atomicAdd(count, 1);

	uint maxPointsPerBuffer = 134 * 1000 * 1000;
	VertexS vs;
	if(vertexID < maxPointsPerBuffer){
		vs = vbo0[vertexID];
	}else if(vertexID < 2 * maxPointsPerBuffer){
		vs = vbo1[vertexID - maxPointsPerBuffer];
	}else if(vertexID < 3 * maxPointsPerBuffer){
		vs = vbo2[vertexID - 2 * maxPointsPerBuffer];
	}else if(vertexID < 4 * maxPointsPerBuffer){
		vs = vbo3[vertexID - 3 * maxPointsPerBuffer];
	}

	VertexT vt;
	vt.ux = vs.ux;
	vt.uy = vs.uy;
	vt.uz = vs.uz;
	vt.color = vs.color;
	vt.index = vertexID;
	//vt.color = 0xFF0000FF;

	targetBuffer[counter] = vt;
	
	//indices[counter] = 8;
	//indices[counter] = vertexID;
	
}






