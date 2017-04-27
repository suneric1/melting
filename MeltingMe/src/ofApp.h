#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "Skeleton.h"

class SkeletonData {
public:
	ofxOscMessage message;
	float time = 0;
};

class Drip {
public:
	ofRectangle rect;
	int a = 255;
	float vel = 0;
	ofColor color = ofColor(255);
	void update(float speed);
	void draw();
	bool bRemove = false;
};

class Energy {
public:
	ofRectangle rect;
	ofVec2f targetPoint;
	int a = 0;
	float vel = 0;
	ofColor color = ofColor(255, 245, 0);
	void update();
	void draw();
	bool bRemove = false;
};

class Pixel {
public:
	ofRectangle rect;
	bool isLitUp = false;
	bool isMelting = false;
	bool isRestoring = false;
	int a = 0;
	ofColor color = ofColor(255, 255, 255);
	void draw();
	void update();
	Drip createDrip(ofColor c);
	float preScale = 0;
};

class ofApp : public ofBaseApp {
public:
	static bool shouldRemoveDrip(const Drip& d);

	void setup();
	void update();
	void draw();

	void parseMessage(ofxOscMessage amsg);
	void saveRecording();
	void loadPlaybackData(string afilePath);
	vector<string> split(const string &s, char delim);

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void updatePixels();
	void detectTouching();

	//    ofEasyCam cam;

	ofxPanel gui;
	bool bHide;
	ofParameter<bool> bDebug;
	ofParameter<bool> bRecording;
	ofParameter<bool> bUseRecordedData;
	ofParameter<bool> selfRestore;
	ofParameter<int> dripCount;
	ofParameter<int> fps;
	ofParameter<float> bodyWidth;
	ofParameter<float> lastft;
	ofParameter<float> dropSpeed;
	ofParameter<float> meltingSpeedBase;
	ofParameter<float> numRows = 120;
	ofParameter<float> numCols = 90;
	ofParameter<float> touchingThresholdBase = 10;
	ofParameter<float> imageScale;
	ofParameter<int> offsetX;
	ofParameter<int> offsetY;

	ofxOscReceiver oscRX;

	string uniqueFilename = "";
	float startRecordingTime = 0;

	vector< SkeletonData > recordingData;
	bool bUseLiveOsc = false;

	vector<SkeletonData> playbackData;
	vector<SkeletonData> playbackDataCached;
	float playbackTimeStart = 0;
	float touchingThreshold = 40;
	float lastColorChangeTime = 0;

	map< string, shared_ptr<Skeleton> > skeletons;

	vector<Pixel> pixels;
	vector<Drip> drips;
	vector<Drip> energies;
};