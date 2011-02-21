/*
 *  AdminPanel.h
 *  No.3 Particle Cloud
 *
 *  Created by Makira on 11/02/18.
 *  Copyright 2011 yesMAYBEno. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxFileDialogOSX.h"

class FileDef {
public:
	string ID;
	string path;	
};

class AdminPanel {

public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void restoreDefault();	
	void openFileDialogueBG(string ID);
	
	ofEvent<FileDef> onFileDialogueBG; 
	ofEvent<int> onClearBG;
	
	// SSAO
	float camerarangex;
	float camerarangey;
	float aoCap;
	float aoMultiplier;
	float depthTolerance;
	float aorange;
	float readDepthVal;
	float aoDivision;
	float baseColSubdivision;
	
	// DoF
	float focus;
	float aspectratiox;
	float aspectratioy;
	float blurclamp;
	float bias;	
	
	// others
	float colScale;
	float colRadius;
	float colAngle;
	float sizeBase;
	
	bool changeBG;
	bool clearBG;
	float BGColor[4];
	
	bool RESTORDEFBTN;
	bool TOGGLEMOTION;

};