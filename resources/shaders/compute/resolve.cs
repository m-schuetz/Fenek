
#version 450

#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_NV_shader_atomic_int64 : enable

layout(local_size_x = 16, local_size_y = 16) in;

struct Vertex{
	float x;
	float y;
	float z;
	uint colors;
};

layout (std430, binding=0) buffer point_data {
	Vertex vertices[];
};

layout (std430, binding=1) buffer framebuffer_data {
	//uint ssFramebuffer[];
	uint64_t ssFramebuffer[];
};

layout(rgba8ui, binding = 0) uniform uimage2D uOutput;

layout(binding = 1) uniform sampler2D uGradient;


uvec4 colorAt(int pixelID){
	uint64_t val64 = ssFramebuffer[pixelID];
	uint ucol = uint(val64 & 0x00FFFFFFUL);
	//uint ucol = ssFramebuffer[pixelID];

	if(ucol == 0){
		return uvec4(0, 0, 0, 255);
	}

	vec4 color = 255.0 * unpackUnorm4x8(ucol);

	//float w = float(ucol) * 0.0001;
	//float w = log2(float(ucol));

	//float w = pow(float(ucol), 0.6) / 500;
	//w = clamp(w, 0, 1);
	//w = -w;

	//float w = log2(float(ucol)) / 15;

	{
		float fcol = float(ucol);
		float w = (log2(fcol) / log2(20)) / 3;
		w = pow(w, 4.2) * 0.4;
		w = clamp(w, 0, 1);
		w = -w;

		//color = clamp(-w * 1, 0.1, 1.0) * 255.0 * texture(uGradient, vec2(w, 0.0));
		color = smoothstep(0.0, 0.1, clamp(-w, 0.03, 1.0)) * 255.0 * texture(uGradient, vec2(w, 0.0));
		//color = 255.0 * texture(uGradient, vec2(w, 0.0));
	}

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
	//ssFramebuffer[pixelID] = 0x00000000;
	//}

}

