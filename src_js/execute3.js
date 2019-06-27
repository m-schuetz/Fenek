

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
	
	//let las = loadLASProgressive("D:/dev/pointclouds/riegl/niederweiden.las");
	//let las = loadLASProgressive("D:/dev/pointclouds/open_topography/ca13/morro_bay.las");
	let las = loadLASProgressive("D:/dev/pointclouds/tu_photogrammetry/wienCity_v3.las");
	
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
		new GLBufferAttribute("value", 1, 4, gl.INT, gl.FALSE, 4, 12, {targetType: "int"}),
		// new GLBufferAttribute("filler", 2, 4, gl.INT, gl.FALSE, 4, 16, {targetType: "int"}),
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


	// {// wien VR
	// 	let s = 0.003;
	// 	pc.transform.elements.set([
	// 	//pc.world.elements.set([
	// 		s, 0, 0, 0, 
	// 		0, 0, s, 0, 
	// 		0, s, 0, 0, 
	// 		-3, 0.8, -1, 1, 
	// 	]);
	// }

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
// 	[172.9393946662685, 22.215277930505863, 254.53787017275238],
// 	[158.7633997583482, 14.629036187845566, 245.55271777061438],
// );

// Wien v3
view.set(
	[99.84371780116061, 257.89267492625004, 499.648918453759],
	[239.06963175530288, -68.43297286478906, 190.54390203032722],
);

// Wien v3 permutes
// view.set(
// 	[245.61471773758868, 40.6343265402732, 319.5790152456486],
// 	[248.38411882796083, 27.02616359615839, 307.4797930398953],
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
// view.set(
// 	[-18.110248752681798, 315.3072958569582, 207.44662796617877],
// 	[343.50616022013264, -10.638824399022553, 502.69411451436576],
// );


// Niederweiden
// view.set(
// 	[19.819479140378164, 9.173117905897247, 20.279376631153855],
// 	[5.526429289048693, 2.4016546381872192, 14.781921727720867],
// );

// Heidentor
// view.set(
// 	[-12.969935350118323, 5.284788162823062, -5.833470159350096],
// 	[-8.397718202711367, 2.8675777530107895, -8.607232645025867],
// );

// view.set(
// 	[-4.870924392952716, 4.87083748499702, -5.880010074273333],
// 	[-8.598512369167397, 3.0739831347414635, -8.409958387065902],
// );
