

if(typeof e4called === "undefined"){
	e4called = true;
	
	let las = loadBINProgressive("D:/dev/pointclouds/riegl/niederweiden_400m.bin");
	//let las = loadLASProgressive("D:/dev/pointclouds/tuwien_baugeschichte/Museum Affandi_las export/batch_1.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/NVIDIA/laserscans/merged.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/hofbibliothek/HB_64.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/tuwien_baugeschichte/Candi Sari_las export/candi_sari.las");
	//let las = loadBINProgressive("D:/dev/pointclouds/test.bin");

	//let las = loadLASProgressive("D:/dev/pointclouds/archpro/heidentor.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/mschuetz/lion.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/tu_photogrammetry/wienCity_v3.las");


	let pc = new PointCloudProgressive("testcloud", "blabla");

	let handles = las.handles;

	let attributes = [
		new GLBufferAttribute("position", 0, 3, gl.FLOAT, gl.FALSE, 12, 0),
		new GLBufferAttribute("color",    1, 4, gl.UNSIGNED_BYTE, gl.TRUE, 4, 12),
		//new GLBufferAttribute("value", 1, 4, gl.INT, gl.FALSE, 4, 12, {targetType: "int"}),
	];

	let bytesPerPoint = attributes.reduce( (p, c) => p + c.bytes, 0);

	let maxPointsPerBuffer = 134 * 1000 * 1000;
	let numPointsLeft = las.numPoints;
	let glBuffers = handles.map( (handle) => {

		let numPointsInBuffer = numPointsLeft > maxPointsPerBuffer ? maxPointsPerBuffer : numPointsLeft;
		numPointsLeft -= maxPointsPerBuffer;

		let glbuffer = new GLBuffer();

		glbuffer.attributes = attributes;

		gl.bindVertexArray(glbuffer.vao);
		glbuffer.vbo = handle;
		gl.bindBuffer(gl.ARRAY_BUFFER, glbuffer.vbo);

		for(let attribute of attributes){

			let {location, count, type, normalize, offset} = attribute;

			gl.enableVertexAttribArray(location);

			if(attribute.targetType === "int"){
				gl.vertexAttribIPointer(location, count, type, bytesPerPoint, offset);
			}else{
				gl.vertexAttribPointer(location, count, type, normalize, bytesPerPoint, offset);
			}
		}

		gl.bindVertexArray(0);

		glbuffer.count =  numPointsInBuffer;

		return glbuffer;
	});

	pc.glBuffers = glBuffers;

	let s = 0.3;
	pc.transform.elements.set([
		s, 0, 0, 0, 
		0, 0, s, 0, 
		0, s, 0, 0, 
		-10, 1.4, -11, 1, 
	]);

	scene.root.add(pc);

	listeners.update.push(() => {

		if(pc.numPoints !== las.numPoints){
			//log(las.numPoints);
		}
		pc.numPoints = las.numPoints;
		
	});

}

view.set(
	[-11.34062835354112, 4.054401647775217, -7.931739384765451],
	[-8.015805951564435, 2.834597543000753, -8.789907181758469],
);

camera.fov = 100;

// log($("testcloud"));

// if($("testcloud")){
// 	let pc = $("testcloud");
// 	let s = 0.03;
// 	pc.transform.elements.set([
// 		s, 0, 0, 0, 
// 		0, 0, s, 0, 
// 		0, s, 0, 0, 
// 		-0.5, 0.7, 1, 1, 
// 	]);

// 	log("lala");

// }