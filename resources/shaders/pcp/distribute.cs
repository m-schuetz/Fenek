#version 450

layout(local_size_x = 32, local_size_y = 1) in;

struct Vertex{
	float ux;
	float uy;
	float uz;
	uint color;
};

layout(std430, binding = 0) buffer ssInputBuffer{
	Vertex inputBuffer[];
};

layout(std430, binding = 2) buffer ssTargetBuffer{
	Vertex targetBuffer[];
};

layout(location = 2) uniform int uNumPoints;
layout(location = 3) uniform double uPrime;
layout(location = 4) uniform int uOffset;

// see https://preshing.com/20121224/how-to-generate-a-sequence-of-unique-random-integers/
double permute(double number, double prime){

	if(number > prime){
		return number;
	}

	double residue = mod(number * number, prime);

	if(number <= prime / 2){
		return residue;
	}else{
		return prime - residue;
	}
}

void main(){
	
	uint inputIndex = gl_GlobalInvocationID.x;

	if(inputIndex >= uNumPoints){
		return;
	}

	uint globalInputIndex = inputIndex + uOffset;

	double p1 = permute(double(globalInputIndex), uPrime);
	double p2 = permute(p1, uPrime);

	uint targetIndex = uint(p2);

	//uint targetIndex = globalInputIndex;

	Vertex v = inputBuffer[inputIndex];

	targetBuffer[targetIndex] = v;
}



