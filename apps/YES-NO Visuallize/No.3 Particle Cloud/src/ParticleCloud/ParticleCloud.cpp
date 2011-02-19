/*
 *  ParticleCloud.cpp
 *  No.3 Particle Cloud
 *
 *  Created by //Makira on 11/02/18.
 *  Copyright 2011 yesMAYBEno. All rights reserved.
 *
 */

#include "ParticleCloud.h"

void ParticleCloud::setup(int _fps, AdminPanel* ap) {

	bullet = new ofxBullet();
	bullet->initPhysics(ofxVec3f(0, 0, 0), false);	
	
	fps = _fps;
	adminPanel = ap;
	yes.setup(bullet, ap, Obj::YES);
	no.setup(bullet, ap, Obj::NO);
	
	flock.setup(ofGetWidth(), ofGetHeight(), 0, -500, 20); // trails for yes and no objs
	
}

void ParticleCloud::update() {

	if (adminPanel->TOGGLEMOTION) {
		
		bullet->stepPhysicsSimulation(fps);
		flock.update(true);
		
		yes.computeShape(flock.getTrailPoints(3));
		yes.update();
		
		no.computeShape(flock.getTrailPoints(14));
		no.update();
	}
	
}

void ParticleCloud::draw() {

//	bullet->render();
	yes.draw();
	no.draw();
	
//	flock.draw();
	
}