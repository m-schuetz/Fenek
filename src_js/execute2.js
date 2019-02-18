


if($("desktop_mirror")){
	let mirror = $("desktop_mirror");

	mirror.position.set(0., 1.15, -0.6);
	mirror.scale.set(0.9, 0.9, 0.9)
	mirror.rotation = new Matrix4().makeRotationY(0.0);
	mirror.updateMatrixWorld();
}

reportState(true);

MSAA_SAMPLES = 4;
EDL_ENABLED = true;
RENDER_DEFAULT_ENABLED = true;
desktopMirrorEnabled = true;

//vr.start();

// window.width = 1600;
// window.height = 1200 - 23;
// window.x  = 2560;
// window.y = 23;

window.width = window.monitorWidth * 0.8;
window.height = window.monitorHeight * 0.8;
window.x = window.monitorWidth * 0.1;
window.y = window.monitorHeight * 0.1;


















