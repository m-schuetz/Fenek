#version 450

#extension GL_ARB_gpu_shader_int64 : enable

layout(local_size_x = 1, local_size_y = 1) in;

struct IndirectCommand{
	uint count;
	uint primCount;
	uint firstIndex;
	uint baseInstance;
};

layout(std430, binding = 1) buffer ssIndirectFillCommands{
	IndirectCommand commands[];
};

layout(std430, binding = 2) buffer ssTimestamps{
	uint64_t tStart;
	uint64_t tNow;
};

uniform float uBudgetMillies;
uniform float uBatchSize;

#define MAX_PER_DRAW 20000000;

void main() {
	
	//uint64_t nanos = tNow - tStart;
	//double milliesConsumed = double(nanos) / double(1000000.0);
	//double milliesRemaining = uBudgetMillies - milliesConsumed;

	//double pointsPerMillies = double(numRendered) / milliesConsumed;

	//double estimatedRemainingBudget = pointsPerMillies * milliesRemaining;
	//estimatedRemainingBudget = 0.8 * estimatedRemainingBudget;


	//if(estimatedRemainingBudget > 1000000){
	//	count = 1000000;
	//}else{

	//}


	//
	//primCount = 1;

	
}






