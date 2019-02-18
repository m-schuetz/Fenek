#version 450

layout(local_size_x = 32, local_size_y = 1) in;

struct Vertex{
	float ux;
	float uy;
	float uz;
	uint colors;
	//float random;
};

struct VertexTarget{
	float ux;
	float uy;
	float uz;
	uint colors_orig;
	//float random;
	//uint colors_avg;
	//float accR;
	//float accG;
	//float accB;
	//float accA;
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
	VertexTarget targetBuffer[];
};

// number of points in the new batch / batchSize
uniform int uNumPoints;

// first unused location in the main VBO
// aka the number of previously added points, excluding the new batch
uniform int uOffset;

void main(){
	
	uint workGroupSize = gl_WorkGroupSize.x * gl_WorkGroupSize.y;

	// [0, batchSize) == [0, uNumPoints)
	uint inputIndex = gl_WorkGroupID.x * workGroupSize 
		+ gl_WorkGroupID.y * gl_NumWorkGroups.x * workGroupSize
		+ gl_LocalInvocationIndex;
		

	uint targetIndex = targetIndices[inputIndex];
	uint sequentialIndex = uOffset + inputIndex;
	
	if(inputIndex >= uNumPoints){
		return;
	}

	//targetBuffer[targetIndex] = inputBuffer[inputIndex];
	

	// If there is already a point at the target location, 
	// move that point to a free spot at the end first
	if(targetIndex < uOffset){
		targetBuffer[sequentialIndex] = targetBuffer[targetIndex];
	}
	
	targetBuffer[targetIndex].ux = inputBuffer[inputIndex].ux;
	targetBuffer[targetIndex].uy = inputBuffer[inputIndex].uy;
	targetBuffer[targetIndex].uz = inputBuffer[inputIndex].uz;

	targetBuffer[targetIndex].colors_orig = inputBuffer[inputIndex].colors;
	//targetBuffer[targetIndex].random = inputBuffer[inputIndex].random;
	//targetBuffer[targetIndex].colors_avg = 0;
	//targetBuffer[targetIndex].accR = 0;
	//targetBuffer[targetIndex].accG = 0;
	//targetBuffer[targetIndex].accB = 0;
	//targetBuffer[targetIndex].accA = 0;

}



