


//if(typeof renderPointCloudOctreeState === "undefined")
//{
//	let ssIndirectCommand = gl.createBuffer();
//	let icBytes = 5 * 4;
//	gl.namedBufferData(ssIndirectCommand, icBytes, new ArrayBuffer(icBytes), gl.DYNAMIC_DRAW);
//
//	let ssIBO = gl.createBuffer();
//	let iboBytes = 10 * 1000 * 1000 * 4;
//	gl.namedBufferData(ssIBO, iboBytes, new ArrayBuffer(iboBytes), gl.DYNAMIC_DRAW);
//
//	renderPointCloudOctreeState = {
//		ssIndirectCommand: ssIndirectCommand,
//		ssIBO: ssIBO,
//		round: 0
//	};
//}

getRenderProgressiveState = function(target){


	if(typeof renderProgressiveMap === "undefined"){
		renderProgressiveMap = new Map();
	}

	//let start = now()

	if(!renderProgressiveMap.has(target)){
		let ssIndirectCommand = gl.createBuffer();
		let icBytes = 5 * 4;
		gl.namedBufferData(ssIndirectCommand, icBytes, new ArrayBuffer(icBytes), gl.DYNAMIC_DRAW);

		let reprojectBuffer = new GLBuffer();

		let vboCapacity = 30 * 1000 * 1000;
		let vboBytes = vboCapacity * 16;

		let buffer = new ArrayBuffer(vboBytes);
		let attributes = [
			new GLBufferAttribute("position", 0, 3, gl.FLOAT, gl.FALSE, 12, 0),
			new GLBufferAttribute("value", 1, 4, gl.INT, gl.FALSE, 4, 12, {targetType: "int"}),
			new GLBufferAttribute("index", 2, 4, gl.INT, gl.FALSE, 4, 16, {targetType: "int"}),
		];
		
		reprojectBuffer.setEmptyInterleaved(attributes, vboBytes);

		let fboPrev = new Framebuffer();

		let state = {
			ssIndirectCommand: ssIndirectCommand,
			reprojectBuffer: reprojectBuffer,
			round: 0, 
			fboPrev: fboPrev,
		};

		renderProgressiveMap.set(target, state);
	}

	//let end = now();
	//let duration = end - start;
	//log(`getRenderProgressiveState(): ${duration}s`);

	return renderProgressiveMap.get(target);
};

renderPointCloudProgressive = function(pointcloud, view, proj, target){

	GLTimerQueries.mark("render-progressive-start");

	let {shReproject, shAdd, csCreateIBO} = pointcloud;
	let state = getRenderProgressiveState(target);

	
	let {ssIndirectCommand, reprojectBuffer, fboPrev} = state;

	let mat32 = new Float32Array(16);
	let transform = new Matrix4();
	let world = pointcloud.transform;
	transform.copy(Matrix4.IDENTITY);
	transform.multiply(proj).multiply(view).multiply(world);
	mat32.set(transform.elements);

	let doUpdates = true;

	//doUpdates = false;
	//doUpdates = true;

	let batchSize = 5 * 1000 * 1000;;

	//batchSize = 0.003 * 1000 * 1000;
	//batchSize = 3 * 1000 * 1000;

	// if(true){
	// 	gl.memoryBarrier(gl.ALL_BARRIER_BITS);
	// 	// taken from https://stackoverflow.com/questions/2901102/how-to-print-a-number-with-commas-as-thousands-separators-in-javascript
	// 	const numberWithCommas = (x) => {
	// 		return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
	// 	}

	// 	let resultBuffer = new ArrayBuffer(4 * 4);
	// 	gl.getNamedBufferSubData(ssIndirectCommand, 0, resultBuffer.byteLength, resultBuffer);
	
	// 	let acceptedCount = new DataView(resultBuffer).getInt32(0, true);
	// 	//log("=====");
	// 	//log("accepted: " + numberWithCommas(acceptedCount));

	// 	let key = `accepted (${pointcloud.name})`;
	// 	log(key + ": " + numberWithCommas(acceptedCount));
	// 	//setDebugValue("accepted", numberWithCommas(acceptedCount));
	// 	//log(numberWithCommas(acceptedCount));
	// }

	if(true){ // REPROJECT
		GLTimerQueries.mark("render-progressive-reproject-start");
		gl.useProgram(shReproject.program);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gradientTexture.type, gradientTexture.handle);
		if(shReproject.uniforms.uGradient){
			gl.uniform1i(shReproject.uniforms.uGradient, 0);
		}

		gl.uniformMatrix4fv(shReproject.uniforms.uWorldViewProj, 1, gl.FALSE, mat32);

		gl.bindVertexArray(reprojectBuffer.vao);
		gl.bindBuffer(gl.DRAW_INDIRECT_BUFFER, ssIndirectCommand);

		gl.drawArraysIndirect(gl.POINTS, 0);

		gl.bindVertexArray(0);

		GLTimerQueries.mark("render-progressive-reproject-end");
		GLTimerQueries.measure("render.progressive.reproject", "render-progressive-reproject-start", "render-progressive-reproject-end", (duration) => {
			let ms = (duration * 1000).toFixed(3);
			setDebugValue("gl.render.progressive.reproject", `${ms}ms`);
		});
	}

	if(true){ // ADD
		GLTimerQueries.mark("render-progressive-add-start");
		gl.useProgram(shAdd.program);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gradientTexture.type, gradientTexture.handle);
		if(shAdd.uniforms.uGradient){
			gl.uniform1i(shAdd.uniforms.uGradient, 0);
		}

		gl.uniformMatrix4fv(shAdd.uniforms.uWorldViewProj, 1, gl.FALSE, mat32);

		let buffers = new Uint32Array([
			gl.COLOR_ATTACHMENT0, 
			gl.COLOR_ATTACHMENT1,
		]);
		gl.drawBuffers(buffers.length, buffers);

		let numPoints = pointcloud.numPoints;
		let numRounds = parseInt(numPoints / batchSize);
		numRounds += (numPoints % batchSize) > 0 ? 1 : 0;

		let localRound = state.round % numRounds;
		//localRound = state.round % 140;
		//localRound = 268 + state.round % 30;

		//pointcloud.numPoints = 260 * 1000 * 1000;

		let start = localRound * batchSize;
		let localBatchSize;
		if (localRound == numRounds - 1) {
			localBatchSize = numPoints - start;
		} else {
			localBatchSize = batchSize;
		}

		// log(localRound)

		let offset = start - (start % 134000000);
		//log((start % 134000000))
		//let offset = (start > 134000000) ? 134000000 : 0;
		//log(offset)
		gl.uniform1i(shAdd.uniforms.uOffset, offset);

		let bufferIndex = 0;
		while(start > 134 * 1000 * 1000){
			bufferIndex++;
			start -= 134 * 1000 * 1000;
		}

		let buffer = pointcloud.glBuffers[bufferIndex];

		gl.bindVertexArray(buffer.vao);

		//if(bufferIndex > 0){
		//start = 1;
		//log(localBatchSize)
		gl.drawArrays(gl.POINTS, start, localBatchSize);
		//}

		gl.bindVertexArray(0);

		GLTimerQueries.mark("render-progressive-add-end");
		GLTimerQueries.measure("render.progressive.add", "render-progressive-add-start", "render-progressive-add-end", (duration) => {
			let ms = (duration * 1000).toFixed(3);
			setDebugValue("gl.render.progressive.add", `${ms}ms`);
		});
	}

	if(true){ // CREATE VBO
		GLTimerQueries.mark("render-progressive-ibo-start");

		gl.useProgram(csCreateIBO.program);

		let indirectData = new Uint32Array([0, 1, 0, 0, 0]);
		gl.namedBufferSubData(ssIndirectCommand, 0, indirectData.byteLength, indirectData);

		gl.bindImageTexture(0, target.textures[1], 0, gl.FALSE, 0, gl.READ_WRITE, gl.RGBA8);

		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, ssIndirectCommand);

		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 2, reprojectBuffer.vbo);

		pointcloud.glBuffers.forEach( (buffer, i) => {
			gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 3 + i, buffer.vbo);
		});


		let localSize = {
			x: 16,
			y: 16,
		};

		let groups = [
			parseInt(1 + target.width / localSize.x),
			parseInt(1 + target.height / localSize.y),
			1
		];

		if(target.samples === 2){
			groups[0] *= 2;
		}else if(target.samples === 4){
			groups[0] *= 2;
			groups[1] *= 2;
		}else if(target.samples === 8){
			groups[0] *= 4;
			groups[1] *= 2;
		}else if(target.samples === 16){
			groups[0] *= 4;
			groups[1] *= 4;
		}

		//log(groups);

		gl.memoryBarrier(gl.ALL_BARRIER_BITS);
		gl.dispatchCompute(...groups);
		gl.memoryBarrier(gl.ALL_BARRIER_BITS);

		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, 0);
		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 2, 0);
		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 3, 0);
		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 4, 0);

		GLTimerQueries.mark("render-progressive-ibo-end");
		GLTimerQueries.measure("render.progressive.ibo", "render-progressive-ibo-start", "render-progressive-ibo-end", (duration) => {
			let ms = (duration * 1000).toFixed(3);
			setDebugValue("gl.render.progressive.ibo", `${ms}ms`);
		});

	}

	if(false){
		gl.memoryBarrier(gl.ALL_BARRIER_BITS);
		// taken from https://stackoverflow.com/questions/2901102/how-to-print-a-number-with-commas-as-thousands-separators-in-javascript
		const numberWithCommas = (x) => {
			return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
		}

		let resultBuffer = new ArrayBuffer(4 * 4);
		gl.getNamedBufferSubData(ssIndirectCommand, 0, resultBuffer.byteLength, resultBuffer);
	
		let acceptedCount = new DataView(resultBuffer).getInt32(0, true);
		//log("=====");
		//log("accepted: " + numberWithCommas(acceptedCount));

		let key = `accepted (${pointcloud.name})`;
		log(key + ": " + numberWithCommas(acceptedCount));
		//setDebugValue("accepted", numberWithCommas(acceptedCount));
		//log(numberWithCommas(acceptedCount));
	}

	if(false){
		gl.memoryBarrier(gl.ALL_BARRIER_BITS);
		// taken from https://stackoverflow.com/questions/2901102/how-to-print-a-number-with-commas-as-thousands-separators-in-javascript
		const numberWithCommas = (x) => {
			return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
		}

		let resultBuffer = new ArrayBuffer(10000 * 4);
		let resI32 = new Uint32Array(resultBuffer);
		gl.getNamedBufferSubData(22, 0, resultBuffer.byteLength, resultBuffer);

		//log(resI32);
		let a = Array.from(resI32);

		let max = Math.max(...a);
		let min = Math.min(...a);

		//log(a)

		//log(new Array(resI32));

		log(`targetCounts - max: ${max}, min: ${min}`);
	
		let acceptedCount = new DataView(resultBuffer).getInt32(0, true);
		//log("=====");
		//log("accepted: " + numberWithCommas(acceptedCount));

		let key = `accepted (${pointcloud.name})`;
		//log(key + ": " + numberWithCommas(acceptedCount));
		//setDebugValue("accepted", numberWithCommas(acceptedCount));
		//log(numberWithCommas(acceptedCount));
	}
	
	gl.useProgram(0);

	fboPrev.setSize(target.width, target.height);
	fboPrev.setNumColorAttachments(target.numColorAttachments);

	gl.blitNamedFramebuffer(target.handle, fboPrev.handle, 
		0, 0, target.width, target.height, 
		0, 0, fboPrev.width, fboPrev.height, 
		gl.COLOR_BUFFER_BIT, gl.LINEAR);

	// let [x, y] = [900, 600];
	// let [w, h] = [200, 200];
	// gl.blitNamedFramebuffer(fboPrev.handle, target.handle, 
	// 	x, y, x + w, y + h, 
	// 	0, 0, 800, 800, 
	// 	gl.COLOR_BUFFER_BIT, gl.NEAREST);

	GLTimerQueries.mark("render-progressive-end");
	GLTimerQueries.measure("render.progressive", "render-progressive-start", "render-progressive-end", (duration) => {
		let ms = (duration * 1000).toFixed(3);
		setDebugValue("gl.render.progressive", `${ms}ms`);
	});

	state.round++;


};

"render_progressive.js"