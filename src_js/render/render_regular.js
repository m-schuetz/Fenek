

if( typeof getRenderRegularState === "undefined"){

	getRenderRegularState = () => {

		if( typeof renderRegularState === "undefined"){
			let fboEDL = new Framebuffer();

			//let path = "../../resources/shaders/edl.cs";
			//let shader = new Shader([{type: gl.COMPUTE_SHADER, path: path}]);
			//shader.watch();
			//let edlShader = shader;

			let vsPath = "../../resources/shaders/edl.vs";
			let fsPath = "../../resources/shaders/edl.fs";
			let edlShader = new Shader([
				{type: gl.VERTEX_SHADER, path: vsPath},
				{type: gl.FRAGMENT_SHADER, path: fsPath},
			]);
			edlShader.watch();

			renderRegularState = {
				fboEDL: fboEDL,
				edlShader: edlShader,
			};
		}

		return renderRegularState;
	}

}

if( typeof getRenderRegularStateSSAO === "undefined"){

	getRenderRegularStateSSAO = () => {

		if( typeof renderRegularStateSSAO === "undefined"){
			let fboSSAO = new Framebuffer();

			let ssaoKernel = new Float32Array(4 * 64);
			for(var i = 0; i < 64; i++) {
				var iKernel = new Vector3(
					Math.random() * 2.0 - 1.0,
					Math.random() * 2.0 - 1.0,
					Math.random());

				iKernel = iKernel.normalize();
				var scalar = i / 64.0;
				scalar = lerp(0.1, 1.0, scalar * scalar);
				iKernel.multiplyScalar(scalar);

				ssaoKernel[4 * i + 0] = iKernel.x;
				ssaoKernel[4 * i + 1] = iKernel.y;
				ssaoKernel[4 * i + 2] = iKernel.z;
				ssaoKernel[4 * i + 3] = 1.0;
			}

			let ssaoNoise = new Float32Array(4 * 4 * 3);
			for(var i = 0; i < 16; i++) {
				ssaoNoise[3 * i + 0] = Math.random() * 2.0 - 1.0;
				ssaoNoise[3 * i + 1] = Math.random() * 2.0 - 1.0;
				ssaoNoise[3 * i + 2] = 0.0;
			}

			let noiseSSAO = gl.createTexture();
			gl.bindTexture(gl.TEXTURE_2D, noiseSSAO);
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB16F, 4, 4, 0, gl.RGB, gl.FLOAT, ssaoNoise.buffer);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
			gl.bindTexture(gl.TEXTURE_2D, 0);

			let vsPath = "../../resources/shaders/ssao.vs";
			let fsPath = "../../resources/shaders/ssao.fs";
			let ssaoShader = new Shader([
				{type: gl.VERTEX_SHADER, path: vsPath},
				{type: gl.FRAGMENT_SHADER, path: fsPath},
			]);
			ssaoShader.watch();

			let vsPath1 = "../../resources/shaders/simple_blur.vs";
			let fsPath1 = "../../resources/shaders/simple_blur.fs";
			let blurShader = new Shader([
				{type: gl.VERTEX_SHADER, path: vsPath1},
				{type: gl.FRAGMENT_SHADER, path: fsPath1},
			]);
			blurShader.watch();

			let vsPath2 = "../../resources/shaders/ssao_final.vs";
			let fsPath2 = "../../resources/shaders/ssao_final.fs";
			let ssaoFinalShader = new Shader([
				{type: gl.VERTEX_SHADER, path: vsPath2},
				{type: gl.FRAGMENT_SHADER, path: fsPath2},
			]);
			ssaoFinalShader.watch();

			renderRegularStateSSAO = {
				fboSSAO: fboSSAO,
				kernelSSAO: ssaoKernel,
				noiseSSAO: noiseSSAO,
				ssaoShader: ssaoShader,
				blurShader: blurShader,
				ssaoFinalShader: ssaoFinalShader,
			};
		}

		return renderRegularStateSSAO;
	}
}

if( typeof lerp === "undefined"){
	lerp = (a, b, c) => {
		return a + c * (b - a);
	}
}

var renderRegular = function() {

	let start = now();

	gl.enable(gl.DEPTH_TEST);
	//gl.disable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LESS);
	//gl.clipControl(gl.LOWER_LEFT, gl.NEGATIVE_ONE_TO_ONE );
	//gl.clearDepth(1);
	gl.clipControl(gl.LOWER_LEFT, gl.ZERO_TO_ONE);
	gl.clearDepth(0);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.GREATER);

	fbo.setSize(window.width, window.height);
	fbo.setSamples(MSAA_SAMPLES);

	gl.bindFramebuffer(gl.FRAMEBUFFER, fbo.handle);
	let buffers = new Uint32Array([
		gl.COLOR_ATTACHMENT0,
		gl.COLOR_ATTACHMENT1,
		gl.COLOR_ATTACHMENT2,
	]);
	gl.drawBuffers(buffers.length, buffers);
	
	gl.viewport(0, 0, fbo.width, fbo.height);
	gl.clearColor(0, 0, 0, 1);
	//gl.clearColor(1, 1, 1, 1);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	
	let pointsRendered = 0;

	let view = camera.world.getInverse();
	let proj = camera.projectionMatrix;

	

	renderBuffers(view, proj, fbo);

	// Eye Dome Lighting
	if(EDL_ENABLED && getRenderRegularState().edlShader.compiled){

		GLTimerQueries.mark("edl-start");

		let isquad = $("image_space_quad");
		let buffer = isquad.getComponent(GLBuffer);
		
		let state = getRenderRegularState();
		let fboEDL = state.fboEDL;
		let shader = state.edlShader;
		let shader_data = shader.uniformBlocks.shader_data;

		fboEDL.setSize(fbo.width, fbo.height);
		gl.bindFramebuffer(gl.FRAMEBUFFER, fboEDL.handle);

		//gl.clearColor(Math.cos(5 * now()) * 0.1 + 0.5, 0, 0, 1);
		//gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

		gl.useProgram(shader.program);

		let textureType = gl.TEXTURE_2D_MULTISAMPLE;
		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(textureType, fbo.textures[0]);
		gl.uniform1i(shader.uniforms.uColor, 0);

		gl.activeTexture(gl.TEXTURE1);
		gl.bindTexture(textureType, fbo.depth);
		gl.uniform1i(shader.uniforms.uDepth, 1);

		gl.disable(gl.DEPTH_TEST);
		gl.depthMask(false);
		gl.disable(gl.CULL_FACE);

		shader_data.setFloat32Array("screenSize", new Float32Array([camera.width, camera.height]));
		shader_data.setFloat32("time", now());
		shader_data.setFloat32("near", camera.near);
		shader_data.setFloat32("far", camera.far);
		shader_data.setFloat32("edlStrength", 0.4);
		shader_data.setFloat32("msaaSampleCount", fbo.samples);

		shader_data.bind();
		shader_data.submit();

		gl.bindVertexArray(buffer.vao);
		gl.drawArrays(gl.TRIANGLES, 0, buffer.count);
		gl.bindVertexArray(0);

		GLTimerQueries.mark("edl-end");

		//log(buffer.attributes);

		//log(fboEDL.height);

		gl.enable(gl.DEPTH_TEST);
		gl.depthMask(true);
		
		gl.blitNamedFramebuffer(fboEDL.handle, 0, 
			0, 0, fboEDL.width, fboEDL.height, 
			0, 0, window.width, window.height, 
			gl.COLOR_BUFFER_BIT, gl.LINEAR);

		//gl.blitNamedFramebuffer(fboEDL.handle, 0, 
		//	600, 400, 600 + 128, 400 + 128, 
		//	0, 0, 512, 512, 
		//	gl.COLOR_BUFFER_BIT, gl.NEAREST);
	}
	else if(SSAO_ENABLED && getRenderRegularStateSSAO().ssaoShader.compiled){

		// --> DRAW OCCLUSION TO FRAMEBUFFER
		GLTimerQueries.mark("ssao-start");

		gl.bindFramebuffer(gl.FRAMEBUFFER, fbo.handle);
		let buffers = new Uint32Array([
			gl.COLOR_ATTACHMENT3,
		]);
		gl.drawBuffers(buffers.length, buffers);

		let isquad = $("image_space_quad");
		let buffer = isquad.getComponent(GLBuffer);

		let state = getRenderRegularStateSSAO();
		let shader = state.ssaoShader;
		let shader_data = shader.uniformBlocks.shader_data;

		gl.useProgram(shader.program);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, state.noiseSSAO);
		gl.uniform1i(shader.uniforms.uNoise, 0);

		let textureType = gl.TEXTURE_2D_MULTISAMPLE;
		gl.activeTexture(gl.TEXTURE1);
		gl.bindTexture(textureType, fbo.textures[1]);
		gl.uniform1i(shader.uniforms.uPosition, 1);

		gl.activeTexture(gl.TEXTURE2);
		gl.bindTexture(textureType, fbo.textures[2]);
		gl.uniform1i(shader.uniforms.uNormal, 2);

		let ssaoKernel = new Float32Array(3 * 64);
		for(var i = 0; i < 64; i++) {
			var iKernel = new Vector3(
				Math.random() * 2.0 - 1.0,
				Math.random() * 2.0 - 1.0,
				Math.random());

			iKernel = iKernel.normalize();
			var scalar = i / 64.0;
			scalar = lerp(0.1, 1.0, scalar * scalar);
			iKernel.multiplyScalar(scalar);

			ssaoKernel[3 * i + 0] = iKernel.x;
			ssaoKernel[3 * i + 1] = iKernel.y;
			ssaoKernel[3 * i + 2] = iKernel.z;
		}

		gl.disable(gl.DEPTH_TEST);
		gl.depthMask(false);
		gl.disable(gl.CULL_FACE);

		let viewProj = new Matrix4();
		viewProj.multiplyMatrices(camera.projectionMatrix, camera.transform.getInverse());
		shader_data.setFloat32Array("view", camera.transform.getInverse().elements);
		shader_data.setFloat32Array("proj", camera.projectionMatrix.elements);
		shader_data.setFloat32Array("viewProj", viewProj.elements);
		let noiseScale = new Float32Array(2);
		noiseScale[0] = fbo.width / 4.0;
		noiseScale[1] = fbo.height / 4.0;
		shader_data.setFloat32Array("noiseScale", noiseScale);
		shader_data.setFloat32("time", now());
		shader_data.setFloat32("near", camera.near);
		shader_data.setFloat32("far", camera.far);
		shader_data.setFloat32("msaaSampleCount", fbo.samples);
		let camPosition = new Float32Array(4);
		camPosition[0] = camera.position.x;
		camPosition[1] = camera.position.y;
		camPosition[2] = camera.position.z;
		camPosition[3] = 1.0;
		shader_data.setFloat32Array("cameraPosition", camPosition);
		shader_data.setFloat32("radius", 0.1);
		shader_data.setFloat32("bias", 0.05);
		shader_data.setFloat32Array("ssaoSamples[0]", state.kernelSSAO);

		// This is not working currently
		let screenSize = new Float32Array(2);
		screenSize[0] = window.width;
		screenSize[1] = window.height;
		shader_data.setFloat32Array("screenSize", screenSize);

		shader_data.bind();
		shader_data.submit();

		gl.bindVertexArray(buffer.vao);
		gl.drawArrays(gl.TRIANGLES, 0, buffer.count);
		gl.bindVertexArray(0);

		GLTimerQueries.mark("ssao-end");
		// --> END THIS

		// --> START BLUR THE OCCLUSION TEXTURE
		GLTimerQueries.mark("ssao-blur-start");

		shader = state.blurShader;
		gl.bindFramebuffer(gl.FRAMEBUFFER, fbo.handle);
		buffers = new Uint32Array([
			gl.COLOR_ATTACHMENT4,
		]);
		gl.drawBuffers(buffers.length, buffers);

		gl.useProgram(shader.program);

		gl.disable(gl.DEPTH_TEST);
		gl.depthMask(false);
		gl.disable(gl.CULL_FACE);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(textureType, fbo.textures[3]);
		gl.uniform1i(shader.uniforms.uInput, 0);

		gl.bindVertexArray(buffer.vao);
		gl.drawArrays(gl.TRIANGLES, 0, buffer.count);
		gl.bindVertexArray(0);

		GLTimerQueries.mark("ssao-blur-end");
		// --> END THIS

		// --> START JOIN ALL COLOR TEXTURES INTO ONE FINAL FRAMEBUFFER
		GLTimerQueries.mark("ssao-composition-start");

		shader = state.ssaoFinalShader;

		let fboSSAO = state.fboSSAO;
		fboSSAO.setSize(fbo.width, fbo.height);
		gl.bindFramebuffer(gl.FRAMEBUFFER, fboSSAO.handle);
		buffers = new Uint32Array([
			gl.COLOR_ATTACHMENT0,
		]);
		gl.drawBuffers(buffers.length, buffers);

		gl.useProgram(shader.program);

		gl.disable(gl.DEPTH_TEST);
		gl.depthMask(false);
		gl.disable(gl.CULL_FACE);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(textureType, fbo.textures[0]);
		gl.uniform1i(shader.uniforms.uColor, 0);

		gl.activeTexture(gl.TEXTURE1);
		gl.bindTexture(textureType, fbo.textures[4]);
		gl.uniform1i(shader.uniforms.uOcclusion, 1);

		gl.bindVertexArray(buffer.vao);
		gl.drawArrays(gl.TRIANGLES, 0, buffer.count);
		gl.bindVertexArray(0);

		GLTimerQueries.mark("ssao-composition-end");
		// --> END THIS

		gl.enable(gl.DEPTH_TEST);
		gl.depthMask(true);

		gl.blitNamedFramebuffer(fboSSAO.handle, 0,
			0, 0, fboSSAO.width, fboSSAO.height,
			0, 0, window.width, window.height,
			gl.COLOR_BUFFER_BIT, gl.LINEAR);
	}else{
		gl.bindFramebuffer(gl.FRAMEBUFFER, 0);
		gl.blitNamedFramebuffer(fbo.handle, 0, 
			0, 0, fbo.width, fbo.height, 
			0, 0, window.width, window.height, 
			gl.COLOR_BUFFER_BIT, gl.LINEAR);
	}



	
	

	//let duration = now() - start;
	//let durationMS = duration * 1000;
	//log(`${durationMS.toFixed(3)}ms`);

	

}


"render_regular.js"