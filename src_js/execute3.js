

// {
// 	loadLASTest("C:/dev/pointclouds/eclepens.las");
// }

// RENDER_DEFAULT_ENABLED = false;

if(true){
	//let handle = test("C:/dev/pointclouds/heidentor.las");
	//let las = loadLASTest("C:/dev/pointclouds/heidentor.las");
	//let las = loadLASTest("C:/dev/pointclouds/eclepens.las");

	//let las = loadLASProgressive("D:/dev/pointclouds/archpro/heidentor.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/eclepens.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/Riegl/Retz_Airborne_Terrestrial_Combined_1cm.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/tu_photogrammetry/wienCity_v3.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/weiss/pos6_LDHI_module.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/pix4d/eclepens.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/weiss/pos7_Subsea_equipment.las");
	let las = loadLASProgressive("C:/dev/pointclouds/planquadrat/wiener_neustadt_waldschule/HAUS_1.las");
	//let las = loadLASProgressive("C:/dev/pointclouds/wienCity.las");

	let handle = las.handle;

	let pc = new PointCloudProgressive("testcloud", "blabla");

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

	for(let attribute of attributes){

		let {location, count, type, normalize, offset} = attribute;

		gl.enableVertexAttribArray(location);
		gl.vertexAttribPointer(location, count, type, normalize, bytesPerPoint, offset);
	}

	gl.bindVertexArray(0);

	glbuffer.count =  las.numPoints;

	let s = 0.3;
	pc.transform.elements.set([
	//pc.world.elements.set([
		s, 0, 0, 0, 
		0, 0, s, 0, 
		0, s, 0, 0, 
		0, 0, 1, 1, 
	]);

	pc.components.push(glbuffer);

	scene.root.add(pc);

	listeners.update.push(() => {
		glbuffer.count = las.numPoints;
	});

}


// view.set(
// 	[164.42231627935024, -5.900582339455357, 161.40410448358546],
// 	[150.21777325461176, -13.570647286902897, 152.53596373947232],
// );

// view.set(
// 	[305.1064642682852, 184.50874775922932, 243.39670446762017],
// 	[117.4753025533808, 9.854579852929703, 49.53156905880002],
// );

view.set(
	[7.054412950986094, 57.23419229658709, 15.211762493608227],
	[-3.0606188271204786, 51.43034875981343, 19.9299008744527],
);

// view.set(
// 	[34.561273790963696, 5.460729767465222, 27.92798827205126],
// 	[31.237606651110774, 1.556378880387479, 31.850219804959266],
// );

// view.set(
// 	[331.2004960537879, 91.57152257292512, -21.3101997346852],
// 	[218.1819628747063, -40.3160839205064, 112.92660882547818],
// );