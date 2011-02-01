#ifndef _TEST_APP
#define _TEST_APP

#define FPS 25

#include "ofMain.h"
#include "BlobManager.h"
#include "AdminPanel.h"
#include "HTTPClient.h"


// No.1 Organic Blob
class App : public ofBaseApp{
	
public:
	
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void resized(int w, int h);
	
	void onFileChange(FileDef& fd);
	
	BlobManager blobMgr;
	AdminPanel	adminPanel;
	HTTPClient	httpClient;	
	
};

#endif
