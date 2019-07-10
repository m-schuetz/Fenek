

const jsDir = "../../src_js";
const resourceDir = "../../resources";

watchJS(`${jsDir}/math/Vector3.js`);
watchJS(`${jsDir}/math/Vector4.js`);
watchJS(`${jsDir}/math/Matrix4.js`);
watchJS(`${jsDir}/math/Plane.js`);
watchJS(`${jsDir}/defines.js`);
watchJS(`${jsDir}/math/Box3.js`);
watchJS(`${jsDir}/math/Ray.js`);
watchJS(`${jsDir}/PointAttributes.js`);
watchJS(`${jsDir}/PotreeLoader.js`);
watchJS(`${jsDir}/scene/SceneNode.js`);
watchJS(`${jsDir}/scene/Camera.js`);
watchJS(`${jsDir}/scene/MeshNode.js`);
watchJS(`${jsDir}/scene/Scene.js`);
watchJS(`${jsDir}/GL.js`);
watchJS(`${jsDir}/scene/Mesh.js`);
watchJS(`${jsDir}/Framebuffer.js`);
watchJS(`${jsDir}/OrbitControls.js`);
watchJS(`${jsDir}/View.js`);
watchJS(`${jsDir}/utils.js`);
watchJS(`${jsDir}/libs/BinaryHeap.js`);
watchJS(`${jsDir}/vr.js`);

watchJS(`${jsDir}/OBJLoader.js`);
watchJS(`${jsDir}/scene/BrushNode.js`);

watchJS(`${jsDir}/render/render.js`);
watchJS(`${jsDir}/render/render_vr.js`);
watchJS(`${jsDir}/render/render_regular.js`);
watchJS(`${jsDir}/render/render_pointcloud_basic.js`);
watchJS(`${jsDir}/render/render_pointcloud_octree.js`);
watchJS(`${jsDir}/render/render_progressive.js`);
watchJS(`${jsDir}/render/render_compute.js`);
watchJS(`${jsDir}/render/render_clod.js`);

watchJS(`${jsDir}/scene/PointCloudOctree.js`);
watchJS(`${jsDir}/scene/PointCloudProgressive.js`);
watchJS(`${jsDir}/scene/PointCloudBasic.js`);
watchJS(`${jsDir}/scene/PointCloudExp.js`);

watchJS(`${jsDir}/math/Intersections.js`);

watchJS(`${jsDir}/Shader.js`);

let fbo = new Framebuffer();
fbo.setNumColorAttachments(2);

let desktopMirrorEnabled = true;

{
	GLMaterial.DEFAULT = new GLMaterial();
	let vsPath = "../../resources/shaders/mesh.vs";
	let fsPath = "../../resources/shaders/mesh.fs";
	let shader = new Shader([
		{type: gl.VERTEX_SHADER, path: vsPath},
		{type: gl.FRAGMENT_SHADER, path: fsPath},
	]);
	shader.watch();
	GLMaterial.DEFAULT.shader = shader;
}

let gradientImage = loadImage(`../../resources/images/gradient_spectral_2d.png`);
let gradientTexture = new GLTexture(gradientImage.width, gradientImage.height, gradientImage.data);

let listeners = {
	update: [],
	render: [],
};


runJSFile(`${jsDir}/scripts/createDefaultScene.js`);
//runJSFile(`${jsDir}/scripts/createScene.js`);
runJSFile(`${jsDir}/scripts/createPointCloudScene.js`);
//runJSFile(`${jsDir}/scripts/createControllers.js`);

//runJSFile(`${jsDir}/scripts/createSpot.js`);
//watchJS(`${jsDir}/scripts/createSpotNew.js`);
//runJSFile(`${jsDir}/scripts/createBlub.js`);

watchJS(`${jsDir}/update.js`);

watchJS(`${jsDir}/execute.js`);
watchJS(`${jsDir}/execute2.js`);
// watchJS(`${jsDir}/subsample/subsample.js`);
// watchJS(`${jsDir}/subsample/subsample_exec.js`);
watchJS(`${jsDir}/execute_drawperf.js`);
monitorJS(`${jsDir}/execute3.js`);
monitorJS(`${jsDir}/execute4.js`);
monitorJS(`${jsDir}/execute5.js`);
monitorJS(`${jsDir}/execute6.js`);

monitorJS(`${jsDir}/pcp/benchmark_heidentor_progressive.js`);
monitorJS(`${jsDir}/pcp/benchmark_hierarchical.js`);














