#version 450

layout(local_size_x = 32, local_size_y = 1) in;

struct Vertex{
	float ux;
	float uy;
	float uz;
	uint color;
};

// This buffer contains points of the newly loaded batch,
// which are going to be distributed to the main VBO by this shader
layout(std430, binding = 0) buffer ssInputBuffer{
	Vertex inputBuffer[];
};

// contains the target location in the main VBO for each point in the new batch
layout(std430, binding = 1) buffer ssTargetIndices{
	uint targetIndices[];
};

// the main VBO. the new batch is distributed over this buffer
layout(std430, binding = 2) buffer ssTargetBuffer{
	Vertex targetBuffer[];
};

// number of points in the new batch / batchSize
layout(location = 2) uniform int uNumPoints;

void main(){
	
	uint inputIndex = gl_GlobalInvocationID.x;

	if(inputIndex >= uNumPoints){
		return;
	}

	uint targetIndex = targetIndices[inputIndex];

	Vertex v = inputBuffer[inputIndex];

	//v.color = 0xFF0000FF;
	//v.uy = 3.0;

	targetBuffer[targetIndex] = v;
	//targetBuffer[0] = v;
	
	
}



