//
//  Skeleton.cpp
//  KinectV2Receive
//
//  Created by Nick Hardeman on 4/5/17.
//
//  Modified by Eric Sun.

#include "Skeleton.h"

//--------------------------------------------------------------
void Skeleton::build() {
	color = (Color)(int)ofRandom(0, 4);
	for (int i = 0; i < TOTAL_JOINTS; i++) {
		addOrUpdateJoint(getNameForIndex((JointIndex)i), ofVec3f(), false);
	}
}

//--------------------------------------------------------------
void Skeleton::draw() {

	ofPolyline l;

	l.addVertex(ofVec2f(getJoint(FOOT_LEFT)->pos));
	l.addVertex(ofVec2f(getJoint(ANKLE_LEFT)->pos));
	l.addVertex(ofVec2f(getJoint(KNEE_LEFT)->pos));
	l.addVertex(ofVec2f(getJoint(HIP_LEFT)->pos));
	l.addVertex(ofVec2f(getJoint(SPINE_BASE)->pos));
	sections["LeftLeg"].line = l;
	l.clear();

	l.addVertex(ofVec2f(getJoint(FOOT_RIGHT)->pos));
	l.addVertex(ofVec2f(getJoint(ANKLE_RIGHT)->pos));
	l.addVertex(ofVec2f(getJoint(KNEE_RIGHT)->pos));
	l.addVertex(ofVec2f(getJoint(HIP_RIGHT)->pos));
	l.addVertex(ofVec2f(getJoint(SPINE_BASE)->pos));
	sections["RightLeg"].line = l;
	l.clear();

	l.addVertex(ofVec2f(getJoint(HAND_TIP_LEFT)->pos));
	l.addVertex(ofVec2f(getJoint(HAND_LEFT)->pos));
	l.addVertex(ofVec2f(getJoint(WRIST_LEFT)->pos));
	l.addVertex(ofVec2f(getJoint(ELBOW_LEFT)->pos));
	l.addVertex(ofVec2f(getJoint(SHOULDER_LEFT)->pos));
	//    l.addVertex( ofVec2f(getJoint(SPINE_SHOULDER)->pos) );
	sections["LeftArm"].line = l;
	l.clear();

	l.addVertex(ofVec2f(getJoint(HAND_TIP_RIGHT)->pos));
	l.addVertex(ofVec2f(getJoint(HAND_RIGHT)->pos));
	l.addVertex(ofVec2f(getJoint(WRIST_RIGHT)->pos));
	l.addVertex(ofVec2f(getJoint(ELBOW_RIGHT)->pos));
	l.addVertex(ofVec2f(getJoint(SHOULDER_RIGHT)->pos));
	//    l.addVertex( ofVec2f(getJoint(SPINE_SHOULDER)->pos) );
	sections["RightArm"].line = l;
	l.clear();

	l.addVertex(ofVec2f(getJoint(SPINE_BASE)->pos));
	l.addVertex(ofVec2f(getJoint(SPINE_MID)->pos));
	l.addVertex(ofVec2f(getJoint(SPINE_SHOULDER)->pos));
	l.addVertex(ofVec2f(getJoint(NECK)->pos));
	l.addVertex(ofVec2f(getJoint(HEAD)->pos));
	sections["Spine"].line = l;
	l.clear();

	restoringSpeed = ofGetLastFrameTime() * 0.5f;

	if (restoring) {
		sections["LeftLeg"].updatePercent(restoringSpeed);
		sections["RightLeg"].updatePercent(restoringSpeed);
		if (sections["LeftLeg"].percentLeft > 0.9f) {
			sections["Spine"].updatePercent(restoringSpeed);
		}
		else {
			sections["Spine"].updatePercent(0);
		}
		if (sections["Spine"].percentLeft > 0.9f) {
			sections["RightArm"].updatePercent(restoringSpeed);
			sections["LeftArm"].updatePercent(restoringSpeed);
		}
		else {
			sections["RightArm"].updatePercent(0);
			sections["LeftArm"].updatePercent(0);
		}
	}
	else {
		sections["Spine"].updatePercent(-meltingSpeed);
		if (sections["Spine"].percentLeft < 0.5f && sections["RightArm"].percentLeft > 0.05f) {
			sections["RightArm"].updatePercent(-meltingSpeed);
			sections["LeftArm"].updatePercent(-meltingSpeed);
		}
		else {
			sections["RightArm"].updatePercent(0);
			sections["LeftArm"].updatePercent(0);
		}
		if (sections["Spine"].percentLeft < 0.1f && sections["LeftLeg"].percentLeft > 0.05f) {
			sections["LeftLeg"].updatePercent(-meltingSpeed);
			sections["RightLeg"].updatePercent(-meltingSpeed);
		}
		else {
			sections["LeftLeg"].updatePercent(0);
			sections["RightLeg"].updatePercent(0);
		}
	}

	    /*for(auto& line: sections){
	        line.second.line.draw();
	    }
	    
	    for( auto& joint : joints ) {
	        ofDrawSphere(ofVec2f(joint.second->pos), 10 );
	    }*/
}

ofColor Skeleton::getColor() {
	switch (color) {
		//        case WHITE:
		//            return ofColor(255);
	case RED:
		return ofColor(247, 89, 89);
		//        case GREEN:
		//            return ofColor(173,247,89);
	case CYAN:
		return ofColor(89, 247, 171);
	case BLUE:
		return ofColor(89, 202, 247);
	case YELLOW:
		return ofColor(247, 239, 89);

	default:
		break;
	}
}

void Skeleton::BodySection::updatePercent(float amount) {
	percentLeft += amount;
	if (percentLeft>1)percentLeft = 1;
	if (percentLeft<0)percentLeft = 0;
	float meltingIndex = line.getIndexAtPercent(percentLeft);
	meltedPoint = line.getPointAtPercent(percentLeft + 0.05f);
	for (int i = 0; i<line.size(); i++) {
		if (i>meltingIndex) {
			line.getVertices()[i] = line.getPointAtPercent(percentLeft);
		}
	}
	//    meltedPoint = line.getVertices()[meltingIndex>=line.size()? (int)meltingIndex:(int)meltingIndex+1];
}

//--------------------------------------------------------------
shared_ptr <Skeleton::Joint> Skeleton::getJoint(string jointName) {
	for (auto joint : joints) {
		if (joint.first == jointName) {
			return joint.second;
		}
	}
	return shared_ptr<Joint>();
}

//--------------------------------------------------------------
shared_ptr <Skeleton::Joint> Skeleton::getJoint(JointIndex aJointIndex) {
	return getJoint(getNameForIndex(aJointIndex));
}

//--------------------------------------------------------------
string Skeleton::getNameForIndex(JointIndex aindex) {
	switch (aindex) {
	case SPINE_BASE:
		return "SpineBase";
	case SPINE_MID:
		return "SpineMid";
	case SPINE_SHOULDER:
		return "SpineShoulder";
	case NECK:
		return "Neck";
	case HEAD:
		return "Head";
	case SHOULDER_LEFT:
		return "ShoulderLeft";
	case ELBOW_LEFT:
		return "ElbowLeft";
	case WRIST_LEFT:
		return "WristLeft";
	case HAND_LEFT:
		return "HandLeft";
	case HAND_TIP_LEFT:
		return "HandTipLeft";
	case THUMB_LEFT:
		return "ThumbLeft";
	case SHOULDER_RIGHT:
		return "ShoulderRight";
	case ELBOW_RIGHT:
		return "ElbowRight";
	case WRIST_RIGHT:
		return "WristRight";
	case HAND_RIGHT:
		return "HandRight";
	case HAND_TIP_RIGHT:
		return "HandTipRight";
	case THUMB_RIGHT:
		return "ThumbRight";
	case HIP_LEFT:
		return "HipLeft";
	case KNEE_LEFT:
		return "KneeLeft";
	case ANKLE_LEFT:
		return "AnkleLeft";
	case FOOT_LEFT:
		return "FootLeft";
	case HIP_RIGHT:
		return "HipRight";
	case KNEE_RIGHT:
		return "KneeRight";
	case ANKLE_RIGHT:
		return "AnkleRight";
	case FOOT_RIGHT:
		return "FootRight";
	default:
		return "Unknown";

	}
	return "Unknown";
}


//--------------------------------------------------------------
void Skeleton::addOrUpdateJoint(string jointName, ofVec3f position, bool seen) {

	if (joints.count(jointName) == 0) {
		//        ofLogError() << " all joints should be made at startup! jointName = " << jointName <<  endl;
		//will crash here - so lets make a shared_ptr
		joints[jointName] = shared_ptr <Joint>(new Joint());
		joints[jointName]->name = jointName;
	}

	scale = ofMap(position.z, 0.f, 4.f, 1000.f, 200.f);
	joints[jointName]->pos = position * scale + ofVec3f(ofGetWidth() / 2, ofGetHeight() * 3 / 5, 0);
	joints[jointName]->bSeen = seen;
	joints[jointName]->bNewThisFrame = true;

	if (firstTimeSeen < 0) {
		firstTimeSeen = lastTimeSeen;
	}
	lastTimeSeen = ofGetElapsedTimef();
}

