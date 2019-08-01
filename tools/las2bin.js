
const fs = require('fs');



//let file = "D:/dev/pointclouds/Riegl/niederweiden.las";
//let targetFile = "D:/dev/pointclouds/Riegl/test.bin";

// RETZ
//let file = "D:/dev/pointclouds/riegl/Retz_Airborne_Terrestrial_Combined_1cm.las";
//let targetFile = "D:/dev/pointclouds/riegl/retz.bin";
// let bytesPerPoint = 34;
// let offsetToPointData = 313;
// let offsetXYZ = 0;
// let offsetRGB = 28;
// let numPoints = 145513439;
// let scale = [0.00025, 0.00025, 0.00025];

// Wien v6 250M
// let file = "D:/dev/pointclouds/tu_photogrammetry/wienCity_v6_250M.las";
// let targetFile = "D:/dev/pointclouds/tu_photogrammetry/wien_v6_250.bin";
// let bytesPerPoint = 88;
// let offsetToPointData = 6198;
// let offsetXYZ = 0;
// let offsetRGB = 30;
// let numPoints = 276667889;
// let scale = [0.001, 0.001, 0.001];

// Wien v6 125
// let file = "D:/dev/pointclouds/tu_photogrammetry/wienCity_v6_125M.las";
// let targetFile = "D:/dev/pointclouds/tu_photogrammetry/wien_v6_125.bin";
// let bytesPerPoint = 88;
// let offsetToPointData = 6198;
// let offsetXYZ = 0;
// let offsetRGB = 30;
// let numPoints = 124122626;
// let scale = [0.001, 0.001, 0.001];

// Wien v6 350 (405M)
let file = "E:/pointclouds/tuwien_photogrammetry/wienCity_v6_350M.las";
let targetFile = "D:/dev/pointclouds/tu_photogrammetry/wien_v6_350M.bin";
let bytesPerPoint = 88;
let offsetToPointData = 6198;
let offsetXYZ = 0;
let offsetRGB = 30;
let numPoints = 404954035;
let scale = [0.001, 0.001, 0.001];


let fd = fs.openSync(file);
let fo = fs.openSync(targetFile, "w");

//let pointsLeft = 200 * 1000 * 1000;
let pointsLeft = numPoints;
let batchSize = 1000 * 1000;
let pointsRead = 0;

while(pointsLeft > 0){

	let batch = Math.min(batchSize, pointsLeft);

	let source = Buffer.alloc(batch * bytesPerPoint);
	let target = Buffer.alloc(batch * 16);

	fs.readSync(fd, source, 0, source.length, offsetToPointData + pointsRead * bytesPerPoint);

	for(let i = 0; i < batch; i++){
		let pOffset = i * bytesPerPoint;

		let ux =source.readInt32LE(pOffset + offsetXYZ + 0);
		let uy =source.readInt32LE(pOffset + offsetXYZ + 4);
		let uz =source.readInt32LE(pOffset + offsetXYZ + 8);

		let x = ux * scale[0];
		let y = uy * scale[1];
		let z = uz * scale[2];

		// let r = parseInt(source.readUInt16LE(pOffset + offsetRGB + 0));
		// let g = parseInt(source.readUInt16LE(pOffset + offsetRGB + 2));
		// let b = parseInt(source.readUInt16LE(pOffset + offsetRGB + 4));

		let r = parseInt(source.readUInt16LE(pOffset + offsetRGB + 0) / 256);
		let g = parseInt(source.readUInt16LE(pOffset + offsetRGB + 2) / 256);
		let b = parseInt(source.readUInt16LE(pOffset + offsetRGB + 4) / 256);

	   target.writeFloatLE(x, 16 * i + 0);
	   target.writeFloatLE(y, 16 * i + 4);
	   target.writeFloatLE(z, 16 * i + 8);
	   target.writeUInt8(r, 16 * i + 12);
	   target.writeUInt8(g, 16 * i + 13);
	   target.writeUInt8(b, 16 * i + 14);

	}
	
	fs.writeSync(fo, target);

	console.log(`pointsLeft: ${pointsLeft}`);
	pointsLeft -= batch;
	pointsRead += batch;
}



