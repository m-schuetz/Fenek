


if(typeof PointCloudBasic === "undefined"){

	PointCloudBasic = class PointCloudBasic extends SceneNode{

		constructor(name, path){
			super(name);

			this.path = path;

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
			numPoints = Math.min(numPoints, 100 * 1000 * 1000);

			// 20

			// 3x8 scale factors
			let sx = headerView.getFloat64(131, true);
			let sy = headerView.getFloat64(139, true);
			let sz = headerView.getFloat64(147, true);

			// 3x8 offsets
			let ox = headerView.getFloat64(155, true);
			let oy = headerView.getFloat64(163, true);
			let oz = headerView.getFloat64(171, true);

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

				log(`stride: ${stride}`);
				log(`bytesPerPoint: ${bytesPerPoint}`);

				gl.bindVertexArray(0);

				glbuffer.count = numPoints;
			}
			this.components.push(glbuffer);


			let rgbOffset = pointDataFormat == 2 ? 20 : 28;

			log(offsetToPointData);
			log(`filesize: ${file.fileSize()}`);

			await file.setReadLocation(offsetToPointData);

			let pointsPerChunk = 1000 * 1000;

			let source = await file.readBytes(pointsPerChunk * pointDataRecordLength);
			let sourceView = new DataView(source);

			let vboChunk = new ArrayBuffer(pointsPerChunk * bytesPerPoint);
			let vboChunkU8 = new Uint8Array(vboChunk);
			let vboChunkView = new DataView(vboChunk);

			let start = now();
			let i_local = 0;
			for(let i = 0; i < numPoints; i++){

				let offsetSource = i_local * pointDataRecordLength;

				let ux = sourceView.getInt32(offsetSource + 0, true);
				let uy = sourceView.getInt32(offsetSource + 4, true);
				let uz = sourceView.getInt32(offsetSource + 8, true);

				let x = ux * sx;
				let y = uy * sy;
				let z = uz * sz;

				vboChunkView.setFloat32(bytesPerPoint * i_local + 0, x, true);
				vboChunkView.setFloat32(bytesPerPoint * i_local + 4, y, true);
				vboChunkView.setFloat32(bytesPerPoint * i_local + 8, z, true);

				let ur = sourceView.getUint16(offsetSource + rgbOffset + 0, true);
				let ug = sourceView.getUint16(offsetSource + rgbOffset + 2, true);
				let ub = sourceView.getUint16(offsetSource + rgbOffset + 4, true);

				let r = ur / 256;
				let g = ug / 256;
				let b = ub / 256;

				vboChunkU8[bytesPerPoint * i_local + 12] = r;
				vboChunkU8[bytesPerPoint * i_local + 13] = g;
				vboChunkU8[bytesPerPoint * i_local + 14] = b;
				vboChunkU8[bytesPerPoint * i_local + 15] = 255;

				i_local++;

				if(i_local === pointsPerChunk && (i + 1) < numPoints){

					let byteOffset = ((i + 1) - pointsPerChunk) * bytesPerPoint;
					let byteLength = i_local * bytesPerPoint;
					gl.namedBufferSubData(glbuffer.vbo, byteOffset, byteLength, vboChunk);

					glbuffer.count = i;
					
					let duration = now() - start;
					log(`duration: ${(1000 * duration).toFixed(3)}`);

					source = await file.readBytes(pointsPerChunk * pointDataRecordLength);
					start = now();

					sourceView = new DataView(source);


					i_local = 0;
				}
			}

			

			


			file.close();

			
		}

	}
}


"PointCloudProgressive.js"
