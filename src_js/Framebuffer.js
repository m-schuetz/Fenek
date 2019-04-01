
class Framebuffer{

	constructor(){
		this.handle = gl.createFramebuffer();
		this.textures = [];
		this.depth = null;

		this.width = 0;
		this.height = 0;
		this.samples = 1;
		this.numColorAttachments = 1;
		this.colorAttachmentsInfo = undefined;

		this.setSize(512, 512);
	}

	setSamples(samples){
		if(samples === this.samples){
			return;
		}

		this.samples = samples;

		this.updateBuffers();
	}

	setSize(width, height){

		if(width === this.width && height === this.height){
			return;
		}

		this.width = width;
		this.height = height;

		this.updateBuffers();
	}

	setNumColorAttachments(numColorAttachments){
		if(numColorAttachments === this.numColorAttachments){
			return;
		}

		this.numColorAttachments = numColorAttachments;

		this.updateBuffers();
	}

	updateBuffers(){
		gl.bindFramebuffer(gl.FRAMEBUFFER, this.handle);

		let width = this.width;
		let height = this.height;
		let samples = this.samples;

		let texType = (samples === 1) ? gl.TEXTURE_2D : gl.TEXTURE_2D_MULTISAMPLE;

		let oldTextures = this.textures;
		let oldDepth = this.depth;

		{ // COLOR
			this.textures = [];
			for(let i = 0; i < this.numColorAttachments; i++){
				let texture = gl.createTexture();

				gl.bindTexture(texType, texture);

				let inter_format = gl.RGBA8;
				let format = gl.RGBA;
				let type = gl.UNSIGNED_BYTE;

				if(this.colorAttachmentsInfo !== undefined) {
					inter_format = this.colorAttachmentsInfo[i].inter_format;
					format = this.colorAttachmentsInfo[i].format;
					type = this.colorAttachmentsInfo[i].type;
				}

				if(samples === 1){
					gl.texParameteri(texType, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
					gl.texParameteri(texType, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

					gl.texImage2D(texType, 0, inter_format, width, height, 0, format, type, 0);
				}else{
					gl.texImage2DMultisample(texType, samples, inter_format, width, height, false);
				}

				gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0 + i, texType, texture, 0);

				this.textures.push(texture);
			}
					
		}

		{ // DEPTH
			this.depth = gl.createTexture();

			gl.bindTexture(texType, this.depth);


			if(samples === 1){
				gl.texParameteri(texType, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
				gl.texParameteri(texType, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

				gl.texImage2D(texType, 0, gl.DEPTH_COMPONENT32, width, height, 0, gl.DEPTH_COMPONENT, gl.FLOAT, 0);
			}else{
				gl.texImage2DMultisample(texType, samples, gl.DEPTH_COMPONENT, width, height, false);
			}

			gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, texType, this.depth, 0);
		}

		for(let texture of oldTextures){
			gl.deleteTexture(texture);
		}

		if(oldDepth){
			gl.deleteTexture(oldDepth);
		}

		gl.bindFramebuffer(gl.FRAMEBUFFER, 0);
		gl.bindTexture(texType, 0);
	}

};