

//vr.start();

// allows you to adjust the mirror depending on your VR room setup
if($("desktop_mirror")){
	let mirror = $("desktop_mirror");

	mirror.position.set(0., 1.15, -0.6);
	mirror.scale.set(0.9, 0.9, 0.9)
	mirror.rotation = new Matrix4().makeRotationY(0.0);
	mirror.updateMatrixWorld();
}

reportState(false);

MSAA_SAMPLES = 4; // MSAA 1 only works if EDL is disable
EDL_ENABLED = true; // Eye-Dome-Lighting. Only currently available form of illumination
RENDER_DEFAULT_ENABLED = true;
desktopMirrorEnabled = true;

{ // set window position
	let monitors = window.monitors;

	if(monitors.length === 1){
		let monitor = monitors[0];

		window.width = monitor.width * 0.8;
		window.height = monitor.height * 0.8;
		window.x = monitor.width * 0.1;
		window.y = monitor.height * 0.1;
	}else{
		// maximize on second monitor, if available

		let monitor = monitors[1];

		window.width = monitor.width;
		window.height = monitor.height - 1;
		window.x = monitors[0].width;
		window.y = 1; // show 1 px of border to give users the chance to drag the window.
	}
}

















