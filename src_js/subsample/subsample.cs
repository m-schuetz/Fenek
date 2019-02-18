#version 450


layout(local_size_x = 128, local_size_y = 1) in;

struct Vertex{
	float x;
	float y;
	float z;
	uint colors;
};

layout(std430, binding = 0) buffer ssInputBuffer{
	Vertex inputBuffer[];
};

layout(std430, binding = 1) buffer ssTargetBuffer{
	Vertex targetBuffer[];
};

layout(std430, binding = 2) buffer ssGrid{
	int gridBuffer[];
};

layout(std430, binding = 3) buffer ssDrawParameters{
	uint  count;
	uint  primCount;
	uint  first;
	uint  baseInstance;
} drawParameters;

layout(std140, binding = 4) uniform shader_data{
	mat4 transform;
	mat4 world;
	mat4 view;
	mat4 proj;

	vec2 screenSize;
	vec4 pivot;
} ssArgs;

layout(location = 22) uniform int uBatchSize;

#define gridSize 512u

void main(){

	uint inputIndex = gl_GlobalInvocationID.x;

	if(inputIndex > uBatchSize){
		return;
	}

	Vertex v = inputBuffer[inputIndex];

	vec3 aPosition = vec3(v.x, v.y, v.z);

	vec3 indexvf = (aPosition / 16) * gridSize;
	uvec3 indexv = uvec3(
		min(indexvf.x, gridSize - 1),
		min(indexvf.y, gridSize - 1),
		min(indexvf.z, gridSize - 1)
	);

	uint index = indexv.x + indexv.y * gridSize + indexv.z * gridSize * gridSize;

	int cellCount = atomicAdd(gridBuffer[index], 1);

	if(cellCount < 1){
		int targetIndex = int(atomicAdd(drawParameters.count, 1));
		targetBuffer[targetIndex] = v;
	}


	//if((inputIndex & 1) == 1){
	//	int targetIndex = int(atomicAdd(drawParameters.count, 1));
	//	//targetBuffer[targetIndex] = v;
	//}

}


