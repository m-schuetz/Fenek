


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

getRenderProgressiveHQState = function(target){

	if(typeof renderProgressiveHQMap === "undefined"){
		renderProgressiveHQMap = new Map();
	}

	if(!renderProgressiveHQMap.has(target)){
		let ssIndirectCommand = gl.createBuffer();
		let icBytes = 5 * 4;
		gl.namedBufferData(ssIndirectCommand, icBytes, new ArrayBuffer(icBytes), gl.DYNAMIC_DRAW);

		let ssIBO = gl.createBuffer();
		let iboBytes = 10 * 1000 * 1000 * 4;
		gl.namedBufferData(ssIBO, iboBytes, new ArrayBuffer(iboBytes), gl.DYNAMIC_DRAW);

		let fboPrev = new Framebuffer();

		let state = {
			ssIndirectCommand: ssIndirectCommand,
			ssIBO: ssIBO,
			round: 0, 
			fboPrev: fboPrev,
		};

		renderProgressiveHQMap.set(target, state);
	}

	return renderProgressiveHQMap.get(target);
};

renderPointCloudProgressiveHQ = function(pointcloud, view, proj, target){

	GLTimerQueries.mark("render-progressive-hq-start");

	let {shReproject, shAdd, csCreateIBO} = pointcloud;
	let state = getRenderProgressiveHQState(target);

	
	let {ssIndirectCommand, ssIBO, fboPrev} = state;

	let buffer = pointcloud.getComponent(GLBuffer);

	if(buffer === null){
		return;
	}

	let numPoints = buffer.count;

	let mat32 = new Float32Array(16);
	let transform = new Matrix4();
	let world = pointcloud.transform;
	transform.copy(Matrix4.IDENTITY);
	transform.multiply(proj).multiply(view).multiply(world);
	mat32.set(transform.elements);

	let doUpdates = true;

	//doUpdates = false;
	//doUpdates = true;

	let batchSize = 1 * 1000 * 1000;
	let pointSize = 1;

	//batchSize = 0.003 * 1000 * 1000;
	//batchSize = 3 * 1000 * 1000;

	if(true){ // REPROJECT
		GLTimerQueries.mark("render-progressive-hq-reproject-start");
		gl.useProgram(shReproject.program);

		let shader_data = shReproject.uniformBlocks.shader_data;
		shader_data.setFloat32Array("transform", transform.elements);
		shader_data.setFloat32("pointSize", pointSize);
		shader_data.bind();
		shader_data.submit();

		//gl.uniformMatrix4fv(shReproject.uniforms.uWorldViewProj, 1, gl.FALSE, mat32);

		gl.bindVertexArray(buffer.vao);

		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ssIBO);
		gl.bindBuffer(gl.DRAW_INDIRECT_BUFFER, ssIndirectCommand);

		//gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 2, buffer.vbo);

		gl.drawElementsIndirect(gl.POINTS, gl.UNSIGNED_INT, 0);
		//gl.drawArrays(gl.POINTS, 0, 1000 * 1000);

		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, 0);
		gl.bindBuffer(gl.DRAW_INDIRECT_BUFFER, 0);
		GLTimerQueries.mark("render-progressive-hq-reproject-end");
	}

	if(false){ // retrieve number of points to reproject
		// taken from https://stackoverflow.com/questions/2901102/how-to-print-a-number-with-commas-as-thousands-separators-in-javascript
		const numberWithCommas = (x) => {
			return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
		}

		let resultBuffer = new ArrayBuffer(4 * 4);
		gl.getNamedBufferSubData(ssIndirectCommand, 0, resultBuffer.byteLength, resultBuffer);
	
		let acceptedCount = new DataView(resultBuffer).getInt32(0, true);

		let key = `accepted (${pointcloud.name})`;
		setDebugValue(key, numberWithCommas(acceptedCount));
	}

	if(doUpdates){ // ADD
		GLTimerQueries.mark("render-progressive-hq-add-start");
		gl.useProgram(shAdd.program);

		let shader_data = shAdd.uniformBlocks.shader_data;
		shader_data.setFloat32Array("transform", transform.elements);
		shader_data.setFloat32("pointSize", pointSize);
		shader_data.bind();
		shader_data.submit();

		gl.memoryBarrier(gl.ALL_BARRIER_BITS);

		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 2, buffer.vbo);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, fboPrev.textures[0]);
		gl.uniform1i(shAdd.uniforms.uPreviousFrame, 0);

		// gl.activeTexture(gl.TEXTURE0 + 1);
		// gl.bindTexture(gl.TEXTURE_2D, fboPrev.textures[1]);
		// gl.uniform1i(shader.uniforms.uPreviousFrameIndices, 1);

		gl.bindImageTexture(2, fboPrev.textures[1], 0, gl.FALSE, 0, gl.READ_WRITE, gl.RGBA8);

		//log(shader.uniforms.uPreviousFrame);

		let buffers = new Uint32Array([
			gl.COLOR_ATTACHMENT0, 
			gl.COLOR_ATTACHMENT1,
		]);
		gl.drawBuffers(buffers.length, buffers);

		let numRounds = parseInt(numPoints / batchSize);
		numRounds += (numPoints % batchSize) > 0 ? 1 : 0;

		let localRound = state.round % numRounds;
		//localRound = 1;

		let start = localRound * batchSize;
		let localBatchSize;
		if (localRound == numRounds - 1) {
			localBatchSize = numPoints - start;
		} else {
			localBatchSize = batchSize;
		}

		gl.bindVertexArray(buffer.vao);

		//gl.depthFunc(gl.LEQUAL);


		
		gl.drawArrays(gl.POINTS, start, localBatchSize);
		//gl.drawArrays(gl.POINTS, 0, 30 * 1000 * 1000);

		let bytesPerPoint = buffer.attributes.reduce( (p, c) => p + c.bytes, 0);
		// log(bytesPerPoint);
		// log(buffer.attributes)

		GLTimerQueries.mark("render-progressive-hq-add-end");
	}

	if(doUpdates){ // CREATE IBO
		GLTimerQueries.mark("render-progressive-hq-ibo-start");

		gl.useProgram(csCreateIBO.program);

		let indirectData = new Uint32Array([0, 1, 0, 0, 0]);
		gl.namedBufferSubData(ssIndirectCommand, 0, indirectData.byteLength, indirectData);

		gl.bindImageTexture(0, target.textures[1], 0, gl.FALSE, 0, gl.READ_WRITE, gl.RGBA8);

		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, ssIndirectCommand);
		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 3, ssIBO);

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

		gl.dispatchCompute(...groups);

		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, 0);
		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 3, 0);

		GLTimerQueries.mark("render-progressive-hq-ibo-end");

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

	GLTimerQueries.mark("render-progressive-hq-end");

	state.round++;


};

"render_progressive_hq.js"