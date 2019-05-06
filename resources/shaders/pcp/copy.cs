
#version 450

layout(local_size_x = 32, local_size_y = 1) in;

struct Vertex{
	float ux; 
	float uy;
	float uz;
	uint rgba;
}


layout(std430, binding = 0) buffer ssInputBuffer{
	Vertex inputBuffer[];
};

layout(std430, binding = 1) buffer ssTargetBuffer{
	Vertex targetBuffer[];
};

uniform int uNumPoints;

uniform int uOffset;


void main(){

	uint sourceIndex = gl_GlobalInvocationID.x;

	if(sourceIndex > uNumPoints){
		return;
	}

	uint targetIndex = sourceIndex + uOffset;

	Vertex v = inputBuffer[sourceIndex];

	targetBuffer[targetIndex] = v;

}



