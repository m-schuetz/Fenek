


if(typeof PointCloudProgressiveHQ === "undefined"){

	PointCloudProgressiveHQ = class PointCloudProgressiveHQ extends SceneNode{

		constructor(name, path){
			super(name);

			this.path = path;

			{ // distribution shader
				let path = "../../resources/shaders/progressive_hq/distribute.cs";
				let shader = new Shader([{type: gl.COMPUTE_SHADER, path: path}]);
				shader.watch();
				this.csDistribute = shader;
			}

			{ // reprojection shader
				let vsPath = "../../resources/shaders/progressive_hq/reproject.vs";
				let fsPath = "../../resources/shaders/progressive_hq/reproject.fs";

				let shader = new Shader([
					{type: gl.VERTEX_SHADER, path: vsPath},
					{type: gl.FRAGMENT_SHADER, path: fsPath},
				]);
				shader.watch();

				this.shReproject = shader;
			}

			{ // add shader
				let vsPath = "../../resources/shaders/progressive_hq/add.vs";
				let fsPath = "../../resources/shaders/progressive_hq/add.fs";

				let shader = new Shader([
					{type: gl.VERTEX_SHADER, path: vsPath},
					{type: gl.FRAGMENT_SHADER, path: fsPath},
				]);
				shader.watch();

				this.shAdd = shader;
			}

			{ // create IBO shader
				let path = "../../resources/shaders/progressive_hq/create_ibo.cs";
				let shader = new Shader([{type: gl.COMPUTE_SHADER, path: path}]);
				shader.watch();
				this.csCreateIBO = shader;
			}

			{ // normal point cloud material 
				let vsPath = "../../resources/shaders/pointcloud_basic.vs";
				let fsPath = "../../resources/shaders/pointcloud.fs";
				
				let shader = new Shader([
					{type: gl.VERTEX_SHADER, path: vsPath},
					{type: gl.FRAGMENT_SHADER, path: fsPath},
				]);
				shader.watch();

				let material = new GLMaterial();
				material.shader = shader;
				this.components.push(material);
			}

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
			let chunkBytesPerPoint = 16;


			let glbuffer = new GLBuffer();
			{
				let attributes = [
					new GLBufferAttribute("position", 0, 3, gl.FLOAT, gl.FALSE, 12, 0),
					new GLBufferAttribute("color_orig", 1, 4, gl.UNSIGNED_BYTE, gl.TRUE, 4, 12),
					//new GLBufferAttribute("random", 2, 1, gl.FLOAT, gl.FALSE, 4, 16),
					new GLBufferAttribute("color_avg", 2, 4, gl.UNSIGNED_BYTE, gl.TRUE, 4, 16),
					//new GLBufferAttribute("color", 1, 4, gl.UNSIGNED_BYTE, gl.TRUE, 4, 12),
					new GLBufferAttribute("acc", 3, 4, gl.FLOAT, gl.FALSE, 16, 20),
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
			let sourceView = new DataView(source);

			let vboChunk = new ArrayBuffer(pointsPerChunk * chunkBytesPerPoint);
			let vboChunkU8 = new Uint8Array(vboChunk);
			let vboChunkView = new DataView(vboChunk);

			let targetIndices = new Uint32Array(pointsPerChunk);
			// for(let i = 0; i < numPoints; i++){
			// 	targetIndices[i] = i;
			// }
			// shuffle(targetIndices);

			let ssInput = gl.createBuffer();
			gl.namedBufferData(ssInput, vboChunk.byteLength, 0, gl.DYNAMIC_DRAW);
			let ssTargetIndices = gl.createBuffer();
			gl.namedBufferData(ssTargetIndices, targetIndices.byteLength, 0, gl.DYNAMIC_DRAW);
			//gl.namedBufferSubData(ssTargetIndices, 0, targetIndices.byteLength, targetIndices);

			let order = new Array(pointsPerChunk).fill(0).map( (a, i) => i);
			order = shuffle(order);

			let start = now();
			let i_local = 0;
			let i_bucket = 0;


			for(let i = 0; i < numPoints; i++){

				let offsetSource = i_local * pointDataRecordLength;
				let j_local = order[i_local];
				let offsetChunk = j_local * chunkBytesPerPoint;

				let ux = sourceView.getInt32(offsetSource + 0, true);
				let uy = sourceView.getInt32(offsetSource + 4, true);
				let uz = sourceView.getInt32(offsetSource + 8, true);

				let x = ux * sx;
				let y = uy * sy;
				let z = uz * sz;

				vboChunkView.setFloat32(offsetChunk + 0, x, true);
				vboChunkView.setFloat32(offsetChunk + 4, y, true);
				vboChunkView.setFloat32(offsetChunk + 8, z, true);

				let ur = sourceView.getUint16(offsetSource + rgbOffset + 0, true);
				let ug = sourceView.getUint16(offsetSource + rgbOffset + 2, true);
				let ub = sourceView.getUint16(offsetSource + rgbOffset + 4, true);

				let r = ur / 256;
				let g = ug / 256;
				let b = ub / 256;

				vboChunkU8[offsetChunk + 12] = r;
				vboChunkU8[offsetChunk + 13] = g;
				vboChunkU8[offsetChunk + 14] = b;
				vboChunkU8[offsetChunk + 15] = 255;

				let targetBucket = Math.round(Math.random() * i_bucket);
				targetIndices[i_local] = targetBucket * pointsPerChunk + i_local;
				//targetIndices[i_local] = i;

				// if(i > 18 * 1000 * 1000 && i < (18 * 1000 * 1000 + 10)){
				// 	log(i_local);
				// 	log(bytesPerPoint * i_local);

				// 	log(`${x}, ${y}, ${z}`);
				// }

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
					

					vboChunk = new ArrayBuffer(pointsPerChunk * chunkBytesPerPoint);
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


"PointCloudProgressiveHQ.js"
