#version 450

layout(local_size_x = 32, local_size_y = 1) in;

struct Vertex{
	float ux;
	float uy;
	float uz;
	uint attribute;
};

layout(std430, binding = 0) buffer ssInputBuffer{
	uint inputBuffer[];
};

layout(std430, binding = 2) buffer ssTargetBuffer0{
	Vertex targetBuffer0[];
};

layout(std430, binding = 3) buffer ssTargetBuffer1{
	Vertex targetBuffer1[];
};

layout(std430, binding = 4) buffer ssTargetBuffer2{
	Vertex targetBuffer2[];
};

layout(std430, binding = 5) buffer ssTargetBuffer3{
	Vertex targetBuffer3[];
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


	if(targetIndex < 134000000){
		uint attribute = inputBuffer[inputIndex];
		Vertex v = targetBuffer0[targetIndex];
		v.attribute = attribute;

		targetBuffer0[targetIndex] = v;
	}else if(targetIndex < 2 * 134000000){
		targetIndex = targetIndex - 134000000;

		uint attribute = inputBuffer[inputIndex];
		Vertex v = targetBuffer0[targetIndex];
		v.attribute = attribute;

		targetBuffer1[targetIndex] = v;
	}else if(targetIndex < 3 * 134000000){
		targetIndex = targetIndex - 2 * 134000000;

		uint attribute = inputBuffer[inputIndex];
		Vertex v = targetBuffer0[targetIndex];
		v.attribute = attribute;

		targetBuffer2[targetIndex] = v;
	}else if(targetIndex < 4 * 134000000){
		targetIndex = targetIndex - 3 * 134000000;

		uint attribute = inputBuffer[inputIndex];
		Vertex v = targetBuffer0[targetIndex];
		v.attribute = attribute;

		targetBuffer3[targetIndex] = v;
	}
}



