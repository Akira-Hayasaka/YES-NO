/*
 *  ConvexHull.h
 *  No.2 Mathmathical Convex Hull
 *
 *  Created by Makira on 11/02/09.
 *  Copyright 2011 yesMAYBEno. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxBullet.h"
#include "YesNoObject.h"
#include "YesNoObjectSoft.h"
#include "AdminPanel.h"
#include "IncomingSMS.h"
#include "StateText.h"
#include "HTTPSMSClient.h"

const int YES = 0;
const int NO = 1;

class ConvexHull {

public:
	
	bool debugRun;
	
	void setup(int _fps, AdminPanel* _adminPanel, StateText* _sText, ofxCamera* _cam);
	void update();
	void draw(int mouseX, int mouseY);
	void keyPressed(int key);
	float addSMS(int YesOrNo);
	void setupGLStuff();
	
	int						fps;
	ofxBullet*				bullet;
	ofxCamera*				cam;	
	AdminPanel*				admin;
	StateText*				sText;
	
	float mc(float num);
	
	YesNoObject				yes;
	YesNoObject				no;
	
	ofxVec3f				yesPoint;
	int						currentYesLevel;
	YesNoObjectSoft			yesSoft;
	ofxVec3f				noPoint;
	int						currentNoLevel;
	YesNoObjectSoft			noSoft;
	

	vector<IncomingSMS*>	insmsYes;
	vector<IncomingSMS*>	insmsNo;
	bool					bNewSMS;
	void onSmsReached(SmsInfo& smsInfo);
	void onSmsCompleted(SmsInfo& smsInfo);
	
	void onTheEnd(int& YesOrNo);
};