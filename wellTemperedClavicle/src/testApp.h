#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "ofxKinect.h"
#include "ofxBox2d.h"
#include "ofxAutoControlPanel.h"

#define SAMPLE_SPACING 15
#define CONTROL_CIRCLE_RADIUS 8

class kineticContour : public ofxCv::Follower<ofPolyline> {
protected:
	float startedDying;
	ofPoint prevZeroPoint = ofPoint(0,0);
	ofPolyline kineticPoly;
	unsigned int index;
public:
	kineticContour()
		:startedDying(0) {
	}
	void setup(const ofPolyline& track);
	void update(const ofPolyline& track);
	void kill();
	void draw();
	ofPolyline getPoly();
	unsigned int getZeroIndex();
};

class kineticJoint {
public:
    kineticJoint();
    kineticJoint(float x, float y, ofxBox2d &box){
		control.fixture.filter.categoryBits = 0x0002;
		control.fixture.filter.maskBits = 0x0001;
		control.setPhysics(3.0, 0.53, 0.1);
		control.setup(box.getWorld(), x, y, CONTROL_CIRCLE_RADIUS);

        b2BodyDef bd;
        b2Body* mouseBody = box.getWorld()->CreateBody(&bd);
        b2MouseJointDef md;
        md.bodyA    = mouseBody;
        md.bodyB    = control.body;
        md.target   = b2Vec2(x/OFX_BOX2D_SCALE, y/OFX_BOX2D_SCALE);
        md.maxForce = 200.0f * control.body->GetMass();
        b2MouseJoint* mouseJoint  = (b2MouseJoint*)box.getWorld()->CreateJoint(&md);
    }

    ~kineticJoint() {
    }

    ofxBox2dCircle control;
    b2MouseJoint* targetJoint;
};

class testApp : public ofBaseApp {
public:

	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);

	void contactStart(ofxBox2dContactArgs &e);
	void contactEnd(ofxBox2dContactArgs &e);

	b2MouseJoint* createMouseJoint(float x, float y, ofxBox2dCircle target);
	int getAnchorIndex (vector <ofPoint> poly);

	ofxKinect kinect;

	ofxCv::ContourFinder contourFinder;
	ofImage depthImg, diffImg;
	ofPixels previous;

	int nearThreshold;
	int farThreshold;

	int angle;

	//Box2D and Physics Stuff
	ofxBox2d box2d;
	vector <ofxBox2dCircle> circles;
	vector <kineticContour> kContours;
	vector <ofPoint> kinectVertices;
	vector <ofxBox2dCircle> controlCircles;
	vector <b2MouseJoint*> polyJoints;
	ofxCv::TrackerFollower<ofPolyline, kineticContour> tracker;

	//Control Panel
	ofxAutoControlPanel panel;
	ofxOscSender sender;
	ofxOscReceiver receiver;

	//Shader Things
	ofShader shader;
};
