

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
	//let las = loadLASProgressive("D:/dev/pointclouds/tuwien_baugeschichte/Candi Sari_las export/candi_sari.las");
	
	let las = loadLASProgressive("D:/dev/pointclouds/open_topography/ca13/morro_bay.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/tu_photogrammetry/wienCity_v3.las");
	
	//let las = loadLASProgressive("D:/dev/pointclouds/weiss/pos6_LDHI_module.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/pix4d/eclepens.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/weiss/pos7_Subsea_equipment.las");
	//let las = loadLASProgressive("C:/dev/pointclouds/planquadrat/wiener_neustadt_waldschule/HAUS_1.las");
	//let las = loadLASProgressive("C:/dev/pointclouds/wienCity.las");

	let pc = new PointCloudProgressive("testcloud", "blabla");

	//let handles = [las.handle0, las.handle1];
	let handles = las.handles;

	let attributes = [
		new GLBufferAttribute("position", 0, 3, gl.FLOAT, gl.FALSE, 12, 0),
		new GLBufferAttribute("value", 1, 4, gl.INT, gl.FALSE, 4, 12, 
			{targetType: "int"}),
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
	//pc.world.elements.set([
		s, 0, 0, 0, 
		0, 0, s, 0, 
		0, s, 0, 0, 
		-10, 1.4, -11, 1, 
	]);


	// wien VR
	//let s = 0.003;
	//pc.transform.elements.set([
	////pc.world.elements.set([
	//	s, 0, 0, 0, 
	//	0, 0, s, 0, 
	//	0, s, 0, 0, 
	//	0, 0.8, 1, 1, 
	//]);

	// retz VR
	// let s = 0.02;
	// pc.transform.elements.set([
	// //pc.world.elements.set([
	// 	s, 0, 0, 0, 
	// 	0, 0, s, 0, 
	// 	0, s, 0, 0, 
	// 	-10, 1.4, -11, 1, 
	// ]);

	//pc.components.push(glbuffer);

	scene.root.add(pc);

	listeners.update.push(() => {
		pc.numPoints = las.numPoints;
	});

}

// Retz
// view.set(
// 	[164.42231627935024, -5.900582339455357, 161.40410448358546],
// 	[150.21777325461176, -13.570647286902897, 152.53596373947232],
// );

// Wien v3
// view.set(
// 	[-123.64041104361256, 257.16964132726406, 325.6114489431626],
// 	[-29.427251694584953, 15.271786917458371, 36.05849198942843],
// );
// debug
// view.set(
// 	[-44.606838623590704, 15.64767041353231, 16.081424471806084],
// 	[-50.174042035595804, 8.035433544859078, 15.454253342850787],
// );

// view.set(
// 	[-264.64801992309486, 39.44561537379605, -181.21156500238834],
// 	[-266.56910214837404, 18.46805479946822, -198.0474965794385],
// );

// // Wien v4
// view.set(
// 	[35.26739672057562, 3.336713034786203, 0.43405646977710255],
// 	[35.21500759471063, -1.0245859003858673, 6.0378602078675],
// );

// Candi Sari
// view.set(
// 	[2.056489772033026, 2.7938523729086424, -42.16144361465408],
// 	[-4.228164574495073, -1.7072518695921084, -33.733158607929035],
// );

// CA 13
// view.set(
// 	[207670.3893571181, 358.8726908716229, 1174647.2048510776],
// 	[208722.5983242525, -366.6060016749077, 1175058.2303216192],
// );
view.set(
	[-18.110248752681798, 315.3072958569582, 207.44662796617877],
	[343.50616022013264, -10.638824399022553, 502.69411451436576],
);
