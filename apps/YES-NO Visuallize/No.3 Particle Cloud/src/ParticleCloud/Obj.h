/*
 *  Obj.h
 *  No.3 Particle Cloud
 *
 *  Created by Makira on 11/02/18.
 *  Copyright 2011 yesMAYBEno. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxBullet.h"

class Obj {

public:
	void setup(ofxBullet* bullet, ofxVec3f pos, int radius, int mass);
	void setForcePoint(ofxVec3f fp){forcePoint = fp;}
	void movetoForcePoint();
	
	MyRigidBody* getBody(){return body;}
	ofxVec3f getObjPos(){return body->getBodyPos();}
	ofxQuaternion getObjRot(){return body->getBodyRotQuat();}
	
	static const int YES = 0;
	static const int NO = 1;	
	
	int size;
	float colAngle;
	float colScale;
	float colRadius;

private:
	MyRigidBody* body;
	ofxVec3f forcePoint;
	
};