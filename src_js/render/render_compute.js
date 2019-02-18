
function renderComputeBasic(node, view, proj, target){

	GLTimerQueries.mark("render-compute-start");

	// TODO support resizing
	let width = 6000;
	let height = 6000;

	if(typeof computeState === "undefined"){

		let pathRender = `../../resources/shaders/compute/render.cs`;
		let pathResolve = `../../resources/shaders/compute/resolve.cs`;

		let csRender = new Shader([{type: gl.COMPUTE_SHADER, path: pathRender}]);
		let csResolve = new Shader([{type: gl.COMPUTE_SHADER, path: pathResolve}]);

		csRender.watch();
		csResolve.watch();

		let numPixels = width * height; // TODO support resizing
		let framebuffer = new ArrayBuffer(numPixels * 8);

		let ssboFramebuffer = gl.createBuffer();
		gl.namedBufferData(ssboFramebuffer, framebuffer.byteLength, framebuffer, gl.DYNAMIC_DRAW);

		let fbo = new Framebuffer();

		computeState = {
			csRender: csRender,
			csResolve: csResolve,
			numPixels: numPixels,
			ssboFramebuffer: ssboFramebuffer,
			fbo: fbo,
		};
	}

	let csRender = computeState.csRender;
	let csResolve = computeState.csResolve;
	let ssboFramebuffer = computeState.ssboFramebuffer;
	let fbo = computeState.fbo;

	//fbo.setSize(target.width * 2, target.height * 2);
	fbo.setSize(target.width, target.height);

	let buffer = node.getComponent(GLBuffer);
	//let buffer = node.buffer;
	let numPoints = buffer.count;

	let mat32 = new Float32Array(16);
	let transform = new Matrix4();
	let world = node.transform;
	transform.copy(Matrix4.IDENTITY);
	transform.multiply(proj).multiply(view).multiply(world);
	mat32.set(transform.elements);



	{ // RENDER

		GLTimerQueries.mark("render-compute-renderpass-start");

		gl.bindFramebuffer(gl.FRAMEBUFFER, 0);

		gl.useProgram(csRender.program);

		//log(transform32);
		gl.uniformMatrix4fv(csRender.uniforms.uTransform, 1, gl.FALSE, mat32);
		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 0, buffer.vbo);
		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, ssboFramebuffer);
		//gl.bindImageTexture(0, target.textures[0], 0, gl.FALSE, 0, gl.READ_WRITE, gl.RGBA8UI);

		let {width, height} = fbo;
		gl.uniform2i(csRender.uniforms.uImageSize, width, height);

		let groups = parseInt(numPoints / 128);
		//groups = 30000;
		gl.dispatchCompute(groups, 1, 1);

		gl.useProgram(0);
		GLTimerQueries.mark("render-compute-renderpass-end");
	}

	{ // RESOLVE
		GLTimerQueries.mark("render-compute-resolvepass-start");
		gl.useProgram(csResolve.program);

		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 0, buffer.vbo);
		gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, ssboFramebuffer);
		gl.bindImageTexture(0, fbo.textures[0], 0, gl.FALSE, 0, gl.READ_WRITE, gl.RGBA8UI);


		let groups = [
			parseInt(1 + fbo.width / 16),
			parseInt(1 + fbo.height / 16),
			1
		];

		gl.dispatchCompute(...groups);

		gl.useProgram(0);
		GLTimerQueries.mark("render-compute-resolvepass-end");
	}
	
	gl.blitNamedFramebuffer(fbo.handle, target.handle, 
		0, 0, fbo.width, fbo.height, 
		0, 0, target.width, target.height, 
		gl.COLOR_BUFFER_BIT, gl.LINEAR);

	GLTimerQueries.mark("render-compute-end");

}

renderPointCloudCompute = renderComputeBasic;

"render_compute.js"