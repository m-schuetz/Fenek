

// {
// 	loadLASTest("C:/dev/pointclouds/eclepens.las");
// }


if(true){
	//let handle = test("C:/dev/pointclouds/heidentor.las");
	let las = loadLASTest("C:/dev/pointclouds/heidentor.las");
	//let las = loadLASTest("C:/dev/pointclouds/eclepens.las");
	let handle = las.handle;

	//let handle = gl.createBuffer();
	//let size = 1000 * 1000 * 16;
	//gl.namedBufferData(handle, size, 0, gl.STREAM_DRAW);

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

	//glbuffer.count = 25836417;
	glbuffer.count = 0;

	let s = 0.3;
	pc.world.elements.set([
		s, 0, 0, 0, 
		0, 0, s, 0, 
		0, s, 0, 0, 
		0, 0, 1, 1, 
	]);

	pc.components.push(glbuffer);

	log("oashfo");

	scene.root.add(pc);

	listeners.update.push(() => {
		//log(las.numPoints);
		glbuffer.count = las.numPoints;
		//glbuffer.count = 1000 * 1000;;
	});

	//let data = new ArrayBuffer(1000 * 1000 * 16);
	//let view = new DataView(data);
	//for(let i = 0; i < 1000 * 1000; i++){

	//	let x = Math.random();
	//	let y = Math.random();
	//	let z = Math.random();

	//	let r = Math.random();
	//	let g = Math.random();
	//	let b = Math.random();
	//	let a = 255;

	//	view.setFloat32(16 * i + 0, x, true);
	//	view.setFloat32(16 * i + 4, y, true);
	//	view.setFloat32(16 * i + 8, z, true);

	//	view.setUint8(16 * i + 12, r);
	//	view.setUint8(16 * i + 13, g);
	//	view.setUint8(16 * i + 14, b);
	//	view.setUint8(16 * i + 15, a);
	//}

	// listeners.update.push(() => {
	// 	//log("adfi");	
	// 	let buffer = glbuffer.vbo;
	// 	let offset = 0;
	// 	let size = data.byteLength;

	// 	for(let i = 0; i < 1000 * 1000; i += 1000){
			
	// 		let pi = Math.random() * 1000 * 1000;
	// 		pi = parseInt(pi);
	// 		pi = Math.min(1000 * 1000 - 1, pi);

	// 		view.setUint8(16 * pi + 12, 255);
	// 		view.setUint8(16 * pi + 13, 0);
	// 		view.setUint8(16 * pi + 14, 0);
	// 		view.setUint8(16 * pi + 15, 255);

	// 	}

	// 	gl.namedBufferSubData(buffer, offset, size, data);

	// });
}