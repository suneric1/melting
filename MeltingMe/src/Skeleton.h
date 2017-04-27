//
//  Skeleton.h
//  KinectV2Receive
//
//  Created by Nick Hardeman on 4/5/17.
//

#pragma once
#include "ofMain.h"

class Skeleton {
public:

	enum JointIndex {
		SPINE_BASE = 0,
		SPINE_MID,
		SPINE_SHOULDER,
		NECK,
		HEAD,
		SHOULDER_LEFT,
		ELBOW_LEFT,
		WRIST_LEFT,
		HAND_LEFT,
		HAND_TIP_LEFT,
		THUMB_LEFT,
		SHOULDER_RIGHT,
		ELBOW_RIGHT,
		WRIST_RIGHT,
		HAND_RIGHT,
		HAND_TIP_RIGHT,
		THUMB_RIGHT,
		HIP_LEFT,
		KNEE_LEFT,
		ANKLE_LEFT,
		FOOT_LEFT,
		HIP_RIGHT,
		KNEE_RIGHT,
		ANKLE_RIGHT,
		FOOT_RIGHT,
		TOTAL_JOINTS
	};

	enum Color {
		//        WHITE = 0,
		RED,
		//        GREEN,
		CYAN,
		BLUE,
		YELLOW
	};

	class Joint {
	public:
		string name = "Default";
		ofVec3f pos, prevPos;
		bool bSeen = false;
		float lastUpdate = 0.0;
		bool bNewThisFrame = false;
	};

	class BodySection {
	public:
		ofPolyline line;
		ofVec3f meltedPoint;
		float percentLeft = 1;
		void updatePercent(float amount);
	};

	void build();
	void draw();
	ofColor getColor();

	shared_ptr <Joint> getJoint(string jointName);
	shared_ptr<Joint> getJoint(JointIndex aJointIndex);
	string getNameForIndex(JointIndex aindex);
	map <string, BodySection > sections;
	float meltingSpeed = 0.002f;
	float restoringSpeed = 0.01f;
	float scale = 0;
	bool hasSameColor = false;

	void addOrUpdateJoint(string jointName, ofVec3f position, bool seen, float imageScale, int offsetX, int offsetY);

	float firstTimeSeen = -1;
	float lastTimeSeen = 0;

	bool restoring;
	Color color = RED;

protected:
	map <string, shared_ptr<Joint> > joints;
	ofMesh drawMesh;


};