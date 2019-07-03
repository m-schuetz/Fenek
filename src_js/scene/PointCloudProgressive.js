


if(typeof PointCloudProgressive === "undefined"){

	PointCloudProgressive = class PointCloudProgressive extends SceneNode{

		constructor(name, path){
			super(name);

			let start = now();

			this.path = path;

			// { // distribution shader
			// 	let path = "../../resources/shaders/progressive/distribute.cs";
			// 	let shader = new Shader([{type: gl.COMPUTE_SHADER, path: path}]);
			// 	shader.watch();
			// 	this.csDistribute = shader;
			// }

			{ // reprojection shader
				let vsPath = "../../resources/shaders/progressive/reproject.vs";
				let fsPath = "../../resources/shaders/progressive/reproject.fs";

				let shader = new Shader([
					{type: gl.VERTEX_SHADER, path: vsPath},
					{type: gl.FRAGMENT_SHADER, path: fsPath},
				]);
				shader.watch();

				this.shReproject = shader;
			}

			{ // add shader
				let vsPath = "../../resources/shaders/progressive/add.vs";
				let fsPath = "../../resources/shaders/progressive/add.fs";

				let shader = new Shader([
					{type: gl.VERTEX_SHADER, path: vsPath},
					{type: gl.FRAGMENT_SHADER, path: fsPath},
				]);
				shader.watch();

				this.shAdd = shader;
			}

			{ // create IBO shader
				let path = "../../resources/shaders/pcp/create_vbo.cs";
				let shader = new Shader([{type: gl.COMPUTE_SHADER, path: path}]);
				shader.watch();
				this.csCreateIBO = shader;
			}

			// { // normal point cloud material 
			// 	let vsPath = "../../resources/shaders/pointcloud_basic.vs";
			// 	let fsPath = "../../resources/shaders/pointcloud.fs";
				
			// 	let shader = new Shader([
			// 		{type: gl.VERTEX_SHADER, path: vsPath},
			// 		{type: gl.FRAGMENT_SHADER, path: fsPath},
			// 	]);
			// 	shader.watch();

			// 	let material = new GLMaterial();
			// 	material.shader = shader;
			// 	this.components.push(material);
			// }

			let duration = now() - start;
			log(`PointCloudProgressive(): ${duration}s`);

			this.load();


		}

		async load(){


			function shuffle(array) {
				let counter = array.length;

				// While there are elements in the array
				while (counter > 0) {
					// Pick a random index
					let index = Math.floor(Math.random() * counter);

					// Decrease counter by 1
					counter--;

					// And swap the last element with it
					let temp = array[counter];
					array[counter] = array[index];
					array[index] = temp;
				}

				return array;
			};

			
			let file = openFile(this.path);

			let headerSize = 227;

			let headerBuffer = await file.readBytes(headerSize);

			let headerView = new DataView(headerBuffer);

			// 4	
			// 2	
			// 2	
			// 4	
			// 2	
			// 2	
			// 8	
			// 1	
			// 1	
			// 32	
			// 32	
			// 2	
			// 2	
			// 2	 Header Size
			// 4	 Offset to Point Data

			let offsetToPointData = headerView.getUint32(96, true);

			// 4	 num var length records
			// 1	 point data format
			let pointDataFormat = headerView.getUint8(104);

			// 2
			let pointDataRecordLength = headerView.getUint16(105, true);

			// 4
			let numPoints = headerView.getUint32(107, true);
			numPoints = Math.min(numPoints, 120 * 1000 * 1000);
			numPoints = Math.min(numPoints, 0.1 * 1000 * 1000);

			// 20

			// 3x8 scale factors
			let sx = headerView.getFloat64(131, true);
			let sy = headerView.getFloat64(139, true);
			let sz = headerView.getFloat64(147, true);

			// 3x8 offsets
			let ox = headerView.getFloat64(155, true);
			let oy = headerView.getFloat64(163, true);
			let oz = headerView.getFloat64(171, true);

			//let bytesPerPoint = 16;
			//let vbo = new ArrayBuffer(numPoints * bytesPerPoint);
			//let vboF32 = new Float32Array(vbo);
			//let vboU8 = new Uint8Array(vbo);


			let bytesPerPoint = 0;


			let glbuffer = new GLBuffer();
			{
				let attributes = [
					new GLBufferAttribute("position", 0, 3, gl.FLOAT, gl.FALSE, 12, 0),
					new GLBufferAttribute("color_orig", 1, 4, gl.UNSIGNED_BYTE, gl.TRUE, 4, 12),
					//new GLBufferAttribute("random", 2, 1, gl.FLOAT, gl.FALSE, 4, 16),
					//new GLBufferAttribute("color_avg", 2, 4, gl.UNSIGNED_BYTE, gl.TRUE, 4, 16),
					//new GLBufferAttribute("color", 1, 4, gl.UNSIGNED_BYTE, gl.TRUE, 4, 12),
					//new GLBufferAttribute("acc", 3, 4, gl.FLOAT, gl.FALSE, 16, 20),
				];

				glbuffer.attributes = attributes;

				bytesPerPoint = attributes.reduce( (p, c) => p + c.bytes, 0);

				gl.bindVertexArray(glbuffer.vao);
				gl.bindBuffer(gl.ARRAY_BUFFER, glbuffer.vbo);
				gl.bufferData(gl.ARRAY_BUFFER, numPoints * bytesPerPoint, 0, gl.DYNAMIC_DRAW);

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

				glbuffer.count = numPoints;
			}
			this.components.push(glbuffer);






			let rgbOffset = pointDataFormat == 2 ? 20 : 28;


			await file.setReadLocation(offsetToPointData);

			
			let pointsPerChunk = 100 * 1000;
			let source = await file.readBytes(pointsPerChunk * pointDataRecordLength);
			let sourceU8 = new Uint8Array(source);
			let sourceU16 = new Uint16Array(source);
			let sourceView = new DataView(source);

			let vboChunk = new ArrayBuffer(pointsPerChunk * bytesPerPoint);
			let vboChunkF32 = new Float32Array(vboChunk);
			let vboChunkU8 = new Uint8Array(vboChunk);
			let vboChunkView = new DataView(vboChunk);

			let targetIndices = new Uint32Array(pointsPerChunk);

			let ssInput = gl.createBuffer();
			gl.namedBufferData(ssInput, vboChunk.byteLength, 0, gl.DYNAMIC_DRAW);
			let ssTargetIndices = gl.createBuffer();
			gl.namedBufferData(ssTargetIndices, targetIndices.byteLength, 0, gl.DYNAMIC_DRAW);



			let order = new Array(pointsPerChunk).fill(0).map( (a, i) => i);
			order = shuffle(order);

			let tmp = new ArrayBuffer(4);
			let tmpU8 = new Uint8Array(tmp);
			let tmpU16 = new Uint16Array(tmp);
			let tmpI32 = new Int32Array(tmp);

			let start = now();
			let i_local = 0;
			let i_bucket = 0;
			for(let i = 0; i < numPoints; i++){

				let offsetSource = i_local * pointDataRecordLength;
				let j_local = order[i_local];

				tmpU8[0] = sourceU8[offsetSource + 0];
				tmpU8[1] = sourceU8[offsetSource + 1];
				tmpU8[2] = sourceU8[offsetSource + 2];
				tmpU8[3] = sourceU8[offsetSource + 3];
				let ux = tmpI32[0];

				tmpU8[0] = sourceU8[offsetSource + 4];
				tmpU8[1] = sourceU8[offsetSource + 5];
				tmpU8[2] = sourceU8[offsetSource + 6];
				tmpU8[3] = sourceU8[offsetSource + 7];
				let uy = tmpI32[0];

				tmpU8[0] = sourceU8[offsetSource + 8];
				tmpU8[1] = sourceU8[offsetSource + 9];
				tmpU8[2] = sourceU8[offsetSource + 10];
				tmpU8[3] = sourceU8[offsetSource + 11];
				let uz = tmpI32[0];


				//let ux = sourceView.getInt32(offsetSource + 0, true);
				//let uy = sourceView.getInt32(offsetSource + 4, true);
				//let uz = sourceView.getInt32(offsetSource + 8, true);

				let x = ux * sx;
				let y = uy * sy;
				let z = uz * sz;

				//vboChunkF32[4 * j_local + 0] = x;
				//vboChunkF32[4 * j_local + 1] = y;
				//vboChunkF32[4 * j_local + 2] = z;
				vboChunkView.setFloat32(bytesPerPoint * j_local + 0, x, true);
				vboChunkView.setFloat32(bytesPerPoint * j_local + 4, y, true);
				vboChunkView.setFloat32(bytesPerPoint * j_local + 8, z, true);

				tmpU8[0] = sourceU8[offsetSource + rgbOffset + 0];
				tmpU8[1] = sourceU8[offsetSource + rgbOffset + 1];
				let ur = tmpU16[0];

				tmpU8[0] = sourceU8[offsetSource + rgbOffset + 2];
				tmpU8[1] = sourceU8[offsetSource + rgbOffset + 3];
				let ug = tmpU16[0];

				tmpU8[0] = sourceU8[offsetSource + rgbOffset + 4];
				tmpU8[1] = sourceU8[offsetSource + rgbOffset + 5];
				let ub = tmpU16[0];

				//let ur = sourceView.getUint16(offsetSource + rgbOffset + 0, true);
				//let ug = sourceView.getUint16(offsetSource + rgbOffset + 2, true);
				//let ub = sourceView.getUint16(offsetSource + rgbOffset + 4, true);

				let r = ur / 256;
				let g = ug / 256;
				let b = ub / 256;


				vboChunkU8[bytesPerPoint * j_local + 12] = r;
				vboChunkU8[bytesPerPoint * j_local + 13] = g;
				vboChunkU8[bytesPerPoint * j_local + 14] = b;
				vboChunkU8[bytesPerPoint * j_local + 15] = 255;

				//vboChunkView.setFloat32(bytesPerPoint * j_local + 16, Math.random(), true);

				//targetIndices[i_local] = order[i];
				//targetIndices[i_local] = i;

				//targetIndices[i_local] = Math.random() * i_bucket * pointsPerChunk + i_local;
				let targetBucket = Math.round(Math.random() * i_bucket);
				targetIndices[i_local] = targetBucket * pointsPerChunk + i_local;
				//targetIndices[i_local] = i;

				i_local++;

				if(i_local === pointsPerChunk && (i + 1) < numPoints){

					gl.useProgram(this.csDistribute.program);

					gl.namedBufferSubData(ssInput, 0, vboChunk.byteLength, vboChunk);
					gl.namedBufferSubData(ssTargetIndices, 0, targetIndices.byteLength, targetIndices);

					gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 0, ssInput);
					gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, ssTargetIndices);
					gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 2, glbuffer.vbo);

					gl.uniform1i(this.csDistribute.uniforms.uNumPoints, pointsPerChunk);
					gl.uniform1i(this.csDistribute.uniforms.uOffset, i_bucket * pointsPerChunk);

					let groups = [parseInt(pointsPerChunk / 32), 1, 1];
					gl.dispatchCompute(...groups);

					gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 0, 0);
					gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 1, 0);
					gl.bindBufferBase(gl.SHADER_STORAGE_BUFFER, 2, 0);

					gl.useProgram(0);

					gl.memoryBarrier(gl.ALL_BARRIER_BITS);

					glbuffer.count = i;

					//gl.bindVertexArray(glbuffer.vao);
					//gl.bindBuffer(gl.ARRAY_BUFFER, glbuffer.vbo);

					//gl.bufferSubData(gl.ARRAY_BUFFER, i * 16, vboChunk.byteLength, vboChunk);

					//gl.bindVertexArray(0);

					let duration = now() - start;
					//log(`duration: ${duration}`);

					source = await file.readBytes(pointsPerChunk * pointDataRecordLength);
					start = now();

					sourceView = new DataView(source);
					sourceU8 = new Uint8Array(source);
					sourceU16 = new Uint16Array(source);
					

					vboChunk = new ArrayBuffer(pointsPerChunk * bytesPerPoint);
					vboChunkF32 = new Float32Array(vboChunk);
					vboChunkU8 = new Uint8Array(vboChunk);
					vboChunkView = new DataView(vboChunk);

					

					i_local = 0;
					i_bucket++;

					//if(i_bucket > 10){
					//	break;
					//}
				}
			}

			

			


			file.close();

			
		}

	}
}


"PointCloudProgressive.js"
