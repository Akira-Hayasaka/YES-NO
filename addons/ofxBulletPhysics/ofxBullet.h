/*
 *  ofxBullet.h
 *  akiraOF17Kemushi
 *
 *  Created by Makira on 10/09/16.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include "ofMain.h"
#include "ofxVectorMath.h"
#include <btBulletDynamicsCommon.h>
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "bulletBaseApp.h"
#include "MySoftBody.h"
#include "MyRigidBody.h"
#include "Ragdoll.h"
#include "ofxBulletStaticUtil.h"


class ofxBullet : public bulletBaseApp {
	
public:
	
	// "pure virtual" member function overrides.
	ofxBullet(){};
	~ofxBullet() { exitPhysics(); };
	
	void					initPhysics(){ initPhysics(ofxVec3f(0,100,0), false); };
	void					initPhysics(ofxVec3f gravity, bool _bEnableCollisionNotification = false);	
	void					exitPhysics();
	void					stepPhysicsSimulation(float framerate);	
	void					render();	
	void					swapBuffers(){};
	void					clientMoveAndDisplay(){};									
	void					updateModifierKeys(){};
	
	void					enableRayCastingMouseInteraction(ofxCamera* _cam);
	void					mouseDragged(ofMouseEventArgs& event);
	void					mousePressed(ofMouseEventArgs& event);
	void					mouseReleased(ofMouseEventArgs& event);
	
	inline btDynamicsWorld* getWorld() { return m_dynamicsWorld; };

	// just make it a btSoftRigidDynamicsWorld please
	virtual const btSoftRigidDynamicsWorld*	getSoftDynamicsWorld() const { return (btSoftRigidDynamicsWorld*) m_dynamicsWorld; }
	virtual btSoftRigidDynamicsWorld* getSoftDynamicsWorld() { return (btSoftRigidDynamicsWorld*) m_dynamicsWorld; }	
	
	
	MyRigidBody*			createGround(ofxVec3f startTrans, ofxVec3f shape, int mass = 0,
										 ofxVec4f color = ofxVec4f(1.0, 1.0, 1.0, 0.5),
										 int bodyType = KINEMATIC_BODY);	
	MyRigidBody*			createBackWall(ofxVec3f startTrans, ofxVec3f shape, int mass = 0,
											ofxVec4f color = ofxVec4f(0.1, 0.1, 0, 0.5),
											int bodyType = KINEMATIC_BODY);	
	MyRigidBody*			createStaticPlane(ofxVec3f startTrans, ofxVec3f shape, int mass = 0,
										 ofxVec4f color = ofxVec4f(1.0, 1.0, 1.0, 0.5),
										 int bodyType = KINEMATIC_BODY);	
	MyRigidBody*			createBox(ofxVec3f startTrans, ofxVec3f boxShape, int mass,
									  ofxVec4f color = ofxVec4f(0.1, 0.1, 0, 0.5),
									  int bodyType = DYNAMIC_BODY);
	MyRigidBody*			createSphere(ofxVec3f startTrans, int radius, int mass,
										  ofxVec4f color = ofxVec4f(0, 0.1, 0.1, 0.5),
										  int bodyType = DYNAMIC_BODY);	
	MyRigidBody*			createCapsule(ofxVec3f startTrans, int radius, int height, int mass,
										  ofxVec4f color = ofxVec4f(0, 0.1, 0.1, 0.5),
										 int bodyType = DYNAMIC_BODY);
	RagDoll*				createRagdoll(const btVector3& startOffset, int scale = 60);
	MySoftBody*				createRope(ofxVec3f from, ofxVec3f len,
									   int res, int fixed,
									   int mass,
									   ofxVec4f color = ofxVec4f(0.6, 0.6, 0.1, 0.5),
									   ofxVec3f gravity = ofxVec3f(0,10,0),
									   MyRigidBody* anchorTgt = NULL);
	MySoftBody*				createEllipsoid(ofxVec3f gravity, ofxVec3f center, ofxVec3f radius, int res);
	MySoftBody*				createCloth(ofxVec3f gravity, ofxVec3f clothShape[4], int resolution, int fix);	
		
	
	ofxVec3f				getSceneCenter();
	void					setSceneCenter(ofxVec3f _center);	
	
	inline void				enableCollisionNotifycation() { bEnableCollisionNotification = true; }
	inline void				disableCollisionNotifycation() { bEnableCollisionNotification = false; }
	vector<ofxVec3f>		getCollisionPointsA();
	vector<ofxVec3f>		getCollisionPointsB(); // how different A and B??
						
	ofxCamera*				cam;
	MyRigidBody*			ground;
	MyRigidBody*			backWall;
	vector<MyRigidBody*>	myRigidVec;			
	vector<MySoftBody*>		mySoftVec;
	vector<RagDoll*>		myRagdollVec;
	
	
protected:
	
	btVector3 getRayTo(int x,int y);
	btTypedConstraint*						m_pickConstraint;		// constraint for mouse picking
	btRigidBody*							pickedBody;						// for deactivation state		
	
	btSequentialImpulseConstraintSolver*	m_solver;
	btDefaultCollisionConfiguration*		m_collisionConfiguration;
	btCollisionDispatcher*					m_dispatcher;
	btBroadphaseInterface*					m_broadphase;
	
	ofxVec3f								sceneCenter;
	
	bool									bEnableCollisionNotification;
	vector<ofxVec3f>						contactsA;
	vector<ofxVec3f>						contactsB;
	
};