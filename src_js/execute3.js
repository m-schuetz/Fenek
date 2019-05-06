

{
	let handle = test();

	let pc = new PointCloudBasic("testcloud", "blabla");

	let glbuffer = new GLBuffer();

	let attributes = [
		new GLBufferAttribute("position", 0, 3, gl.FLOAT, gl.FALSE, 12, 0),
		new GLBufferAttribute("color_orig", 1, 4, gl.UNSIGNED_BYTE, gl.TRUE, 4, 12),
	];

	glbuffer.attributes = attributes;

	bytesPerPoint = attributes.reduce( (p, c) => p + c.bytes, 0);

	gl.bindVertexArray(glbuffer.vao);
	glbuffer.vbo = handle;
	gl.bindBuffer(gl.ARRAY_BUFFER, glbuffer.vbo);

	let stride = attributes.reduce( (a, v) => a + v.bytes, 0);

	for(let attribute of attributes){
		gl.enableVertexAttribArray(attribute.location);
		gl.vertexAttribPointer(
			attribute.location, 
			attribute.count, 
			attribute.type, 
			attribute.normalize, 
			stride, 
			attribute.offset);	
	}

	gl.bindVertexArray(0);

	glbuffer.count = 2 * 1000 * 1000;

	let s = 10.6;
	pc.transform.elements.set([
		s, 0, 0, 0, 
		0, 0, s, 0, 
		0, s, 0, 0, 
		0, 0, 1, 1, 
	]);

	pc.components.push(glbuffer);

	log("oashfo");

	scene.root.add(pc);
}