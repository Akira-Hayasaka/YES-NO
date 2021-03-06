#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxEasyCam.h"
#include "ObjManager.h"

#define NUM_SPHERES 1

class testApp : public ofSimpleApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed  (int key);
		void keyReleased (int key);
		
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased();
		
		void myInit();
		
		ofxEasyCam camera;
		ofxLight light;
		
		ObjManager* objManager;
};

#endif
	
