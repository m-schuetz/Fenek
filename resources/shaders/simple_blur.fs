#version 450

layout(location = 0) out vec4 outBlur;

layout(binding = 0) uniform sampler2DMS uInput;

const int numSamples = 4;

void main() {
	ivec2 pos 		= ivec2(gl_FragCoord.xy);
	vec4 f_blur 	= vec4(0,0,0,1);

    float result 	= 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y) 
        {
			for(int msaaSampleNr = 0;
					msaaSampleNr < numSamples;
					msaaSampleNr++) {
				ivec2 offset 	= ivec2(x,y);
				result 			+= texelFetch(uInput, pos + offset, msaaSampleNr).r;
			}
        }
    }

	result 	= (result / numSamples) / (4.0 * 4.0);
    outBlur = vec4(result);
}