#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofxShader.h"
#include "ofxDepthFBO.h"
#include "ofxColorFBO.h"
#include "ofx3DUtils.h"
#include "ofxEasyCam.h"

class testApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();

	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	ofxShader defaultShader;
	ofxShader showDepthShader;
	ofxShader ssaoShader;
	ofxDepthFBO depthFBO;
	ofxColorFBO colorFBO;
	
	ofxEasyCam cam;
	
	void drawScene();
	vector<ofxVec4f> objCol;
	vector<ofxVec3f> objPos;
	vector<int> objSize;
	int numObj;	
	void drawFullScreenQuad(int w, int h);

	void setupGLStuff();
	GLfloat *materialAmbient;
	GLfloat *materialDiffuse;
	GLfloat *materialSpecular;	

};

#endif
