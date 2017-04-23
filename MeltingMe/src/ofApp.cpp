#include "ofApp.h"
/*
Address: /bodies/{bodyId}/joints/{jointId}
Values:
- float:  positionX
- float:  positionY
- float:  positionZ
- string: trackingState (Tracked, NotTracked or Inferred)

Address: /bodies/{bodyId}/hands/{handId} (Left or Right)
Values:
- string: handState (Open, Closed, NotTracked, Unknown)
- string: handConfidence (High, Low)

*/

//--------------------------------------------------------------
bool ofApp::shouldRemoveDrip(const Drip& d) {
	return d.bRemove;
}

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);
	ofBackground(30);

	bUseLiveOsc = true;
	// uncomment to use OSC //
	if (bUseLiveOsc) {
		oscRX.setup(12345);
	}

	ofDirectory tdir;
	tdir.allowExt("txt");
	tdir.listDir("recordings");
	if (tdir.size()) {
		loadPlaybackData(tdir.getPath(tdir.size() - 1));
	}


	gui.setup("Image Processing");
	gui.setPosition(ofGetWidth() - 10 - gui.getWidth(), 10);
	gui.add(bDebug.set("Debug", true));
	gui.add(selfRestore.set("Self Restore", true));
	gui.add(dripCount.set("Line Count", 0));
	gui.add(lastft.set("Delta Time", 0));
	gui.add(fps.set("FPS", 0));
	gui.add(touchingThresholdBase.set("Touch Thrd", 10, 5, 20));
	if (bUseLiveOsc) gui.add(bRecording.set("Recording", false));

	float numRows = 160;
	float numCols = 90;
	for (int i = 0; i<numRows; i++) {
		for (int j = 0; j<numCols; j++) {
			Pixel p;
			ofRectangle r;
			r.width = ofGetWidth() / numRows - 2;
			r.height = ofGetHeight() / numCols - 2;
			r.x = ofGetWidth() / numRows*i + 1;
			r.y = ofGetHeight() / numCols*j + 1;
			p.rect = r;
			pixels.push_back(p);
		}
	}
}

//--------------------------------------------------------------
void ofApp::update() {

	float etimef = ofGetElapsedTimef();

	//change color every 10 seconds
	if (etimef - lastColorChangeTime > 10) {
		for (auto it = skeletons.begin(); it != skeletons.end(); it++) {
			it->second->color = (Skeleton::Color)(int)ofRandom(0, 4);
		}
		lastColorChangeTime = etimef;
	}

	for (auto it = skeletons.begin(); it != skeletons.end(); it++) {
		for (int i = 0; i < Skeleton::TOTAL_JOINTS; i++) {
			it->second->getJoint((Skeleton::JointIndex)i)->prevPos = it->second->getJoint((Skeleton::JointIndex)i)->pos;
		}
	}


	if (bUseLiveOsc) {
		while (oscRX.hasWaitingMessages()) {
			ofxOscMessage msg;
			oscRX.getNextMessage(msg);

			parseMessage(msg);

			if (bRecording) {
				if (uniqueFilename == "") {
					uniqueFilename = ofGetTimestampString();
					startRecordingTime = etimef;
					recordingData.clear();
				}
			}
			else {
				// save the file //
				if (uniqueFilename != "") {
					saveRecording();
				}

				recordingData.clear();
				uniqueFilename = "";
			}

			if (bRecording) {
				SkeletonData sdata;
				sdata.time = etimef - startRecordingTime;
				sdata.message = msg;
				recordingData.push_back(sdata);
			}

		}
	}
	else {
		if (playbackData.size() == 0 && playbackDataCached.size()) {
			playbackData = playbackDataCached;
			playbackTimeStart = etimef;
		}

		if (playbackData.size()) {
			float playEndTime = playbackTimeStart + playbackData.back().time;
			float timeSinceStart = etimef - playbackTimeStart;
			int numToKill = 0;
			for (int k = 0; k < playbackData.size(); k++) {
				if (playbackData[k].time <= timeSinceStart) {
					parseMessage(playbackData[k].message);
					numToKill++;
				}
				else {
					break;
				}
			}
			if (numToKill > 0) {
				playbackData.erase(playbackData.begin(), playbackData.begin() + numToKill);
			}
		}


	}

	// clean up old skeletons //
	for (auto it = skeletons.begin(); it != skeletons.end(); it++) {
		if (etimef - it->second->lastTimeSeen > 2.0) {
			skeletons.erase(it);
			break;
		}
	}

	//    cout << "Number of skeletons : " << skeletons.size() << " | " << ofGetFrameNum() << endl;

	updatePixels();

	dripCount = drips.size();
	lastft = ofGetLastFrameTime();
	fps = ofGetFrameRate();

	for (int i = 0; i<drips.size(); i++) {
		drips[i].update();
	}

	ofRemove(drips, shouldRemoveDrip);

	detectTouching();
}

//--------------------------------------------------------------
void ofApp::detectTouching() {
	for (auto it = skeletons.begin(); it != skeletons.end(); it++) {
		it->second->restoring = false;
		it->second->meltingSpeed = ofGetLastFrameTime() * 0.1f;
		it->second->hasSameColor = false;
		for (auto jt = skeletons.begin(); jt != skeletons.end(); jt++) {
			if (it!=jt && it->second->color == jt->second->color) {
				it->second->hasSameColor = true;
			}
		}
	}

	for (auto it = skeletons.begin(); it != skeletons.end(); it++) {
		touchingThreshold = it->second->scale / touchingThresholdBase;
		for (auto jt = skeletons.begin(); jt != skeletons.end(); jt++) {
			if (it != jt) {
				if (it->second->getJoint("HandLeft")->pos.distance(jt->second->getJoint("HandLeft")->pos) < touchingThreshold
					|| it->second->getJoint("HandRight")->pos.distance(jt->second->getJoint("HandRight")->pos) < touchingThreshold
					|| it->second->getJoint("HandLeft")->pos.distance(jt->second->getJoint("HandRight")->pos) < touchingThreshold
					|| it->second->getJoint("HandRight")->pos.distance(jt->second->getJoint("HandLeft")->pos) < touchingThreshold) {
					if (it->second->color == jt->second->color) {
						it->second->restoring = true;
						jt->second->restoring = true;
					}
					else {
						it->second->meltingSpeed = ofGetLastFrameTime() * 0.4f;
						jt->second->meltingSpeed = ofGetLastFrameTime() * 0.4f;
					}
				}
			}
		}
		if (it->second->getJoint("HandRight")->pos.distance(it->second->getJoint("HandLeft")->pos) < touchingThreshold) {
			if(it->second->hasSameColor)
				it->second->meltingSpeed = ofGetLastFrameTime() * 0.4f;
			else
				it->second->restoring = true;
		}
	}
}

//--------------------------------------------------------------
void ofApp::parseMessage(ofxOscMessage amsg) {

	//    cout << "msg: " << amsg.getAddress() << " | " << ofGetFrameNum() << endl;

	string address = amsg.getAddress();
	if (address.size() > 0 && address[0] == '/') {
		address = address.substr(1, address.size() - 1);
	}
	vector< string > parts = split(address, '/');

	if (parts.size() >= 4) {
		string bodyId = parts[1];
		string typeName = parts[2];
		if (typeName == "joints") {

			ofVec3f tpos;
			tpos.x = amsg.getArgAsFloat(0);
			tpos.y = -amsg.getArgAsFloat(1);
			tpos.z = amsg.getArgAsFloat(2);
			//tpos.z = 0;

			string status = amsg.getArgAsString(3);
			bool bSeen = (status != "NotTracked" && status != "Unknown");

			string jointName = parts[3];

			if (!skeletons.count(bodyId)) {
				skeletons[bodyId] = shared_ptr<Skeleton>(new Skeleton());
				skeletons[bodyId]->build();
			}
			skeletons[bodyId]->addOrUpdateJoint(jointName, tpos, bSeen);
		}


		//duplicate
		//bodyId = parts[1] + "a";
		//if (typeName == "joints") {

		//	ofVec3f tpos;
		//	tpos.x = 1 + amsg.getArgAsFloat(0);
		//	tpos.y = -amsg.getArgAsFloat(1);
		//	tpos.z = amsg.getArgAsFloat(2);
		//	//tpos.z = 0;

		//	string status = amsg.getArgAsString(3);
		//	bool bSeen = (status != "NotTracked" && status != "Unknown");

		//	string jointName = parts[3];

		//	if (!skeletons.count(bodyId)) {
		//		skeletons[bodyId] = shared_ptr<Skeleton>(new Skeleton());
		//		skeletons[bodyId]->build();
		//	}
		//	skeletons[bodyId]->addOrUpdateJoint(jointName, tpos, bSeen);
		//}
	}

}

//--------------------------------------------------------------
void ofApp::draw() {

	ofSetColor(120);
	for (auto it = skeletons.begin(); it != skeletons.end(); it++) {
		it->second->draw();
	}

	for (int i = 0; i < pixels.size(); i++) {
		pixels[i].draw();
	}

	for (int i = 0; i<drips.size(); i++) {
		drips[i].draw();
	}


	if (!bHide) {
		gui.draw();
	}
}

void ofApp::updatePixels() {
	for (int i = 0; i<pixels.size(); i++) {
		pixels[i].isLitUp = false;
		pixels[i].isMelting = false;
		pixels[i].isRestoring = false;
		pixels[i].preScale = 0;
	}

	for (auto it = skeletons.begin(); it != skeletons.end(); it++) {
		for (auto line = it->second->sections.begin(); line != it->second->sections.end(); line++) {
			float sectionWidth = 0.05f * it->second->scale * pow(line->second.percentLeft, 1 / 4.f);
			if (line->first == "Spine") sectionWidth *= 2;
			if (line->second.percentLeft < 0.95f) {
				for (int i = 0; i < pixels.size(); i++) {
					if (it->second->scale >= pixels[i].preScale && line->second.meltedPoint.distance(pixels[i].rect.getCenter())<sectionWidth) {
						pixels[i].color = it->second->getColor();
						pixels[i].preScale = it->second->scale;
						if (it->second->restoring)
							pixels[i].isRestoring = true;
						else {
							if (!((line->first == "LeftLeg" || line->first == "RightLeg" || line->first == "RightArm" || line->first == "LeftArm") && line->second.percentLeft <= 0.05f))
								pixels[i].isMelting = true;
						}
					}
				}
			}
			for (int i = 0; i < pixels.size(); i++) {
				if (it->second->scale >= pixels[i].preScale && line->second.line.getClosestPoint(pixels[i].rect.getCenter()).distance(pixels[i].rect.getCenter()) < sectionWidth) {
					pixels[i].color = it->second->getColor();
					pixels[i].isRestoring = false;
					pixels[i].isMelting = false;
					pixels[i].isLitUp = true;
					pixels[i].a = ofMap(it->second->scale, 400, 200, 255, 180);
					pixels[i].preScale = it->second->scale;
				}
			}
		}
	}

	for (int i = 0; i<pixels.size(); i++) {
		pixels[i].update();
		if (pixels[i].isMelting && (int)(ofGetElapsedTimef()*10)%2==0 ) {
			drips.push_back(pixels[i].createDrip(pixels[i].color));
		}
		if (pixels[i].isRestoring) {
			pixels[i].a = 255;
			pixels[i].color = pixels[i].color.lerp(ofColor(255), 0.5f);
		}
	}
}

void Pixel::update() {
	a -= 50;
	if (a<0)a = 0;
	/*if (isLitUp) {
		a = 255;
	}*/
}

void Pixel::draw() {
	ofSetColor(color, a);
	ofDrawRectangle(rect);
}

Drip Pixel::createDrip(ofColor c) {
	Drip d;
	d.rect = rect;
	d.color = d.color.lerp(c, 0.3f);
	return d;
}

void Drip::update() {
	vel += ofGetLastFrameTime() * 8.f;
	rect.position.y += vel;
	a -= ofGetLastFrameTime() * 300;
	if (a<0) {
		a = 0;
		bRemove = true;
	}
}

void Drip::draw() {
	ofSetColor(color, a);
	ofDrawRectangle(rect);
}

//--------------------------------------------------------------
void ofApp::saveRecording() {
	cout << "Saving recording to " << uniqueFilename << endl;
	if (!ofDirectory::doesDirectoryExist("recordings/")) {
		ofDirectory::createDirectory("recordings/");
	}

	ofBuffer buffer;
	for (auto a : recordingData) {

		buffer.append(ofToString(a.time) + "|");
		buffer.append(a.message.getAddress());

		for (int i = 0; i < a.message.getNumArgs(); i++) {
			buffer.append("|");
			if (a.message.getArgType(i) == OFXOSC_TYPE_FLOAT) {
				buffer.append("f" + ofToString(a.message.getArgAsFloat(i), 6));
			}
			else if (a.message.getArgType(i) == OFXOSC_TYPE_STRING) {
				buffer.append("s" + a.message.getArgAsString(i));
			}
			else if (a.message.getArgType(i) == OFXOSC_TYPE_INT32) {
				buffer.append("i" + ofToString(a.message.getArgAsInt(i)));
			}
			else {
				buffer.append("u0");
			}
		}
		buffer.append("\n");
	}

	ofBufferToFile("recordings/" + uniqueFilename + ".txt", buffer);
	recordingData.clear();
}

//--------------------------------------------------------------
void ofApp::loadPlaybackData(string afilePath) {
	playbackDataCached.clear();

	ofBuffer tbuffer = ofBufferFromFile(afilePath);
	if (tbuffer.size()) {
		for (auto a : tbuffer.getLines()) {
			string lineStr = a;
			ofStringReplace(lineStr, "\n", "");
			vector <string> results = split(lineStr, '|');

			if (results.size() >= 5) {
				SkeletonData sdata;
				sdata.time = ofToFloat(results[0]);
				sdata.message.setAddress(results[1]);
				// now set the params //
				for (int k = 2; k < results.size(); k++) {
					if (results[k] == "") continue;
					if (results[k].length() <= 1) continue;
					string vstring = results[k];
					vstring = vstring.substr(1, vstring.length());
					if (results[k][0] == 'f') {
						// float
						sdata.message.addFloatArg(ofToFloat(vstring));
					}
					else if (results[k][0] == 's') {
						// string //
						sdata.message.addStringArg(vstring);
					}
					else if (results[k][0] == 'i') {
						sdata.message.addIntArg(ofToInt(vstring));
					}
				}
				playbackDataCached.push_back(sdata);
			}
		}
	}
}

//--------------------------------------------------------------
vector<string> ofApp::split(const string &s, char delim) {
	stringstream ss(s);
	string item;
	vector<string> tokens;
	while (getline(ss, item, delim)) {
		tokens.push_back(item);
	}
	return tokens;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == 'h') {
		bHide = !bHide;
	}
	if (key == 'd') {
		bDebug = !bDebug;
	}
	if (key == ' ') {
		bRecording = !bRecording;
	}
	if (key == 's') {
		gui.saveToFile("settings.xml");
	}
	if (key == 'l') {
		gui.loadFromFile("settings.xml");
	}
	if (key == 'r') {
		for (auto it = skeletons.begin(); it != skeletons.end(); it++) {
			for(auto section=it->second->sections.begin(); section!=it->second->sections.end();section++){
			    section->second.percentLeft = 1.f;
			}
			it->second->restoring = true;
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

	if (key == 'r') {
		for (auto it = skeletons.begin(); it != skeletons.end(); it++) {
			it->second->restoring = false;
		}
	}
	if (key == 'c') {
		for (auto it = skeletons.begin(); it != skeletons.end(); it++) {
			it->second->color = (Skeleton::Color)(int)ofRandom(0, 4);
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
