
var gridSize = 512;
var gridData = new Int32Array(gridSize ** 3);
gridData.fill(0);

if( typeof getSubsampleState === "undefined"){

	getSubsampleState = () => {

		if( typeof subsampleState === "undefined"){

			let ssTarget = gl.createBuffer();
			let ssGrid = gl.createBuffer();
			let ssDrawParameters = gl.createBuffer();

			let targetSize = 26 * 1000 * 1000 * 16;
			gl.namedBufferData(ssTarget, targetSize, 0, gl.DYNAMIC_READ);

			// grid
			//let gridData = new Uint8Array(4 * (gridSize ** 3));
			//gridData.fill(0);
			gl.namedBufferData(ssGrid, gridData.byteLength, gridData, gl.DYNAMIC_READ);

			// draw params
			let bufferDrawParameters = new ArrayBuffer(4 * 4);
			gl.namedBufferData(ssDrawParameters, bufferDrawParameters.byteLength, bufferDrawParameters, gl.DYNAMIC_READ);

			let shaderSource = "../../src_js/subsample/subsample.cs";
			let shader = new Shader([{type: gl.COMPUTE_SHADER, path: shaderSource}]);
			shader.watch();

			let setValueShaderSource = "../../resources/shaders/experimental/setValue.cs";
			let setValueShader = new Shader([{type: gl.COMPUTE_SHADER, path: setValueShaderSource}]);
			setValueShader.watch();

			subsampleState = {
				ssTarget: ssTarget,
				ssGrid: ssGrid,
				ssDrawParameters: ssDrawParameters,
				shader: shader,
				setValueShader: setValueShader,
			};
		}

		

	
		return subsampleState;
	};
}



subsample = function(node){

	let glbuffer = node.getComponent(GLBuffer);
	let numPoints = glbuffer.count;

	let state = getSubsampleState();

	let ssSource = glbuffer.vbo;
	let ssTarget = state.ssTarget;
	let ssGrid = state.ssGrid;
	let ssDrawParameters = state.ssDrawParameters;

	let shader = state.shader;
	let setValueShader = state.setValueShader;
	
	let drawParameters = new Int32Array([0, 1, 0, 0]);
	gl.namedBufferSubData(ssDrawParameters, 0, drawParameters.byteLength, drawParameters.buffer);


	//gl.namedBufferSubData(ssGrid, 0, gridData.byteLength, gridData.buffer);

	GLTimerQueries.mark("subsample-start");
	{
		//let shaderSource = "../../resources/shaders/experimental/setValue.cs";
		//let shader = new Shader([{type: gl.COMPUTE_SHADER, path: shaderSource}]);
		//shader.watch();

		let shader = setValueShader;
		let shader_data = shader.uniformBlocks.shader_data;
		gl.useProgram(shader.program);

		shader_data.setInt32("count", gridData.length);
		shader_data.setInt32("value", 0);
		
		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 0, ssGrid);

		shader_data.bind();
		shader_data.submit();

		let groups = [parseInt(gridData.length / 128), 1, 1];

		log(groups);

		gl.memoryBarrier(gl.ALL_BARRIER_BITS);
		gl.dispatchCompute(...groups);
		gl.memoryBarrier(gl.ALL_BARRIER_BITS);

		gl.useProgram(0);

	}

	gl.useProgram(shader.program);

	let shader_data = shader.uniformBlocks.shader_data;

	gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 0, ssSource);
	gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, ssTarget);
	gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 2, ssGrid);
	gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 3, ssDrawParameters);

	gl.uniform1i(22, numPoints);

	//shader_data.setFloat32Array("transform", transform.elements);
	//shader_data.setFloat32Array("world", world.elements);
	//shader_data.setFloat32Array("view", view.elements);
	//shader_data.setFloat32Array("proj", proj.elements);
	//shader_data.setFloat32Array("screenSize", new Float32Array([cam.size.width, cam.size.height]));
	//shader_data.setFloat32Array("pivot", new Float32Array([cpos.x, cpos.y, cpos.z, 0.0]));
	//shader_data.setFloat32("CLOD", CLOD);
	//shader_data.setFloat32("spacing", pointcloud.spacing * 0.8);
	//shader_data.setFloat32("scale", scale);
	//shader_data.setFloat32("time", now());

	//shader_data.bind();
	//shader_data.submit();

	let groups = [parseInt(glbuffer.count / 128), 1, 1];

	
	gl.memoryBarrier(gl.ALL_BARRIER_BITS);
	gl.dispatchCompute(...groups);
	gl.memoryBarrier(gl.ALL_BARRIER_BITS);
	GLTimerQueries.mark("subsample-end");


	{ // read results
		// taken from https://stackoverflow.com/questions/2901102/how-to-print-a-number-with-commas-as-thousands-separators-in-javascript
		const numberWithCommas = (x) => {
			return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
		}

		let resultBuffer = new ArrayBuffer(4 * 4);
		gl.getNamedBufferSubData(ssDrawParameters, 0, resultBuffer.byteLength, resultBuffer);
	
		let acceptedCount = new DataView(resultBuffer).getInt32(0, true);

		log(`accepted ${numberWithCommas(acceptedCount)} out of ${numberWithCommas(glbuffer.count)}`);
		setDebugValue("accepted", `accepted ${numberWithCommas(acceptedCount)} out of ${numberWithCommas(glbuffer.count)}`);
	}

	gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 0, 0);
	gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, 0);
	gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 2, 0);
	gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 3, 0);

	gl.useProgram(0);

	//gl.deleteBuffers(3, new Uint32Array([ssTarget, ssGrid, ssDrawParameters]));

}











