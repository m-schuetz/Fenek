
loadOctree = function(){
	
	let pc = new PointCloudOctree("octree", "D:/dev/pointclouds/converted/affandi_batch_1/cloud.js");
	
	//heidentor.transform.elements.set([
	//	1, 0, 0, 0, 
	//	0, 0, -1, 0, 
	//	0, 1, 0, 0, 
	//	0, 0, 0, 1, 
	//]);
	let s = 1.0;
	pc.transform.elements.set([
		s, 0, 0, 0, 
		0, 0, -s, 0, 
		0, s, 0, 0, 
		0, 0, 0, 1, 
	]);
	scene.root.add(pc);

	view.set(
		[-7.330047391982175, 7.897543503270976, -4.463023058403868],
		[4.437738969389951, 4.55472018779445, -7.284232739227429]
	);
}

loadOctree();

// {
// 	let node = $("heidentor_oct");

// 	log(node.root.children[0]);

// }