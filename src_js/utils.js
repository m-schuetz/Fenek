

function debugSphere(parent, position, scale){
	let n = 50;

	let numPoints = n * n;
	let vertices = new Float32Array(numPoints * 4);
	let verticesU8 = new Uint8Array(vertices.buffer);
	let bytesPerPoint = 16;

	let vindex = 0;
	for(let i = 0; i < n; i++){
		for(let j = 0; j < n; j++){

			let u = 4 * Math.PI * i / n - Math.PI;
			let v = 4 * Math.PI * j / n - Math.PI;

			let radius = 1;
			let x = radius * Math.sin(u) * Math.sin(v); // + 0.05 * Math.cos(20 * v);
			let y = radius * Math.cos(u) * Math.sin(v); // + 0.05 * Math.cos(20 * u);
			let z = radius * Math.cos(v);

			let r = (u + Math.PI) / 2 * Math.PI;
			let g = (v + Math.PI) / 2 * Math.PI;
			let b = 0;

			[r, g, b] = [0.1, 0.22, 0.02];
			r = 255 * (x + radius) / (2 * radius);
			g = 255 * (y + radius) / (2 * radius);
			b = 255 * (z + radius) / (2 * radius);

			r = r < 200 ? 0 : r;
			g = g < 200 ? 0 : g;
			b = b < 200 ? 0 : b;

			vertices[4 * vindex + 0] = x;
			vertices[4 * vindex + 1] = y;
			vertices[4 * vindex + 2] = z;

			verticesU8[16 * vindex + 12] = r;
			verticesU8[16 * vindex + 13] = g;
			verticesU8[16 * vindex + 14] = b;
			verticesU8[16 * vindex + 15] = 255;

			vindex++;
		}
	}

	let vsPath = "../../resources/shaders/mesh.vs";
	let fsPath = "../../resources/shaders/mesh.fs";

	let shader = new Shader([
		{type: gl.VERTEX_SHADER, path: vsPath},
		{type: gl.FRAGMENT_SHADER, path: fsPath},
	]);
	shader.watch();

	let material = new GLMaterial();
	material.glDrawMode = gl.TRIANGLES;
	material.shader = shader;

	let sphere = new SceneNode("sphere_" + Math.random());
	let sphereBuffer = new GLBuffer();
	sphere.components.push(material, sphereBuffer);	

	parent.add(sphere);

	let attributes = [
		new GLBufferAttribute("position", 0, 3, gl.FLOAT, gl.FALSE, 12, 0),
		new GLBufferAttribute("color", 1, 3, gl.UNSIGNED_BYTE, gl.TRUE, 4, 12),
	];
	
	sphereBuffer.set(vertices, attributes, vindex);

	sphere.position.copy(position);
	sphere.scale.set(scale, scale, scale);
	sphere.updateMatrixWorld();

	return sphere;
}

class GLTimerQueries{

	constructor(){
		this.enabled = true;
	}

	static mark(name){

		if(!this.enabled){
			return;
		}

		let query = gl.createQuery();

		gl.queryCounter(query, gl.TIMESTAMP);

		GLTimerQueries.marks.set(name, query);
	}

	static resolve(callback){

		if(!this.enabled){
			return;
		}


		GLTimerQueries.queue.push({
			marks: GLTimerQueries.marks,
			callback: callback,
		});

		GLTimerQueries.marks = new Map();

		let newQueue = [];

		for(let entry of GLTimerQueries.queue){

			let timestamps = new Map();

			for(let [name, query] of entry.marks){
				let timestampAvailable = gl.getQueryObjectui64(query, gl.QUERY_RESULT_AVAILABLE) === gl.TRUE;

				if(timestampAvailable){
					let timestamp = gl.getQueryObjectui64(query, gl.QUERY_RESULT);
					
					timestamps.set(name, timestamp);
				}
			}

			if(timestamps.size === entry.marks.size){
				// all queries fullfilled

				for(let [name, query] of entry.marks){
					gl.deleteQuery(query);
				}

				entry.callback(timestamps);
			}else{
				newQueue.push(entry);
			}

		}

		GLTimerQueries.queue = newQueue;

	}

};


GLTimerQueries.marks = new Map();
GLTimerQueries.queue = [];


$ = (name) => {
	let result = null;

	scene.root.traverse( (node, level) => {
		if(node.name === name){
			result = node;
		}

		let carryOn = result === null;

		return carryOn;
	});

	return result;
}