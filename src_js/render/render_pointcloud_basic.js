
renderPointCloudBasic = function(pointcloud, view, proj, target){

	GLTimerQueries.mark("render-pointcloud-basic-start");

	let material = pointcloud.getComponent(GLMaterial);
	let shader = material.shader;

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

	let pointSize = 2;
	
	{
		gl.useProgram(shader.program);

		let shader_data = shader.uniformBlocks.shader_data;
		shader_data.setFloat32Array("transform", transform.elements);
		shader_data.setFloat32("pointSize", pointSize);
		shader_data.bind();
		shader_data.submit();

		let buffers = new Uint32Array([
			gl.COLOR_ATTACHMENT0, 
			gl.COLOR_ATTACHMENT1,
		]);
		gl.drawBuffers(buffers.length, buffers);

		gl.bindVertexArray(buffer.vao);
		
		gl.drawArrays(gl.POINTS, 0, numPoints);

	}
	
	gl.useProgram(0);

	GLTimerQueries.mark("render-pointcloud-basic-end");
};

"render_pointcloud_basic.js"