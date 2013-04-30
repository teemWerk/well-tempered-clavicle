#include "testApp.h"
#include <algorithm>

#define DYING_TIME 0.1

using namespace ofxCv;
using namespace cv;


void kineticContour::setup(const ofPolyline& track) {
    kineticPoly = track;
    kineticPoly = kineticPoly.getResampledByCount(SAMPLE_SPACING);
    prevZeroPoint = kineticPoly[0];
    index = 0;
}

void kineticContour::update(const ofPolyline& track) {
    kineticPoly = track;
    kineticPoly = kineticPoly.getResampledByCount(SAMPLE_SPACING);
    prevZeroPoint = kineticPoly.getClosestPoint(prevZeroPoint, &index);
}

void kineticContour::kill() {
	dead=true;
}

void kineticContour::draw() {
}

ofPolyline kineticContour::getPoly() {
    return kineticPoly;
}

unsigned int kineticContour::getZeroIndex() {
    /*if(index > kineticPoly.size()-1) {
        return 0;
    }*/
    return index;
}


//--------------------------------------------------------------
void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);

	//Kinect Calibration
	kinect.setRegistration(true);
	kinect.init(false, false); // disable video image (faster fps)
	kinect.open();		// opens first available kinect
	  // zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);

    //ofxCv Setup
	contourFinder.setMinAreaRadius(25);
	contourFinder.getTracker().setPersistence(15);
	contourFinder.getTracker().setMaximumDistance(32);
	//contourFinder.setFindHoles(true);
	//contourFinder.setMaxAreaRadius(150);

     //ofImage buffer allocation
	//depthImg.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	imitate(depthImg, kinect.getDepthPixelsRef());
	imitate(previous, depthImg);
	imitate(diffImg, depthImg);

	//ofxBox2d Setup
	box2d.init();
	box2d.setGravity(0,10);
	//box2d.createBounds();
	box2d.registerGrabbing();
	box2d.setFPS(20.0);
	//box2d.registerGrabbing();
	ofAddListener(box2d.contactStartEvents, this, &testApp::contactStart);
	ofAddListener(box2d.contactEndEvents, this, &testApp::contactEnd);

    //ofxControlPanel Setup
	panel.setup(320, 800);
	panel.setPosition(ofGetWidth()-320,0);
	panel.setDraggable(false);
	panel.addPanel("theWellTemperedClavicle");
	panel.addToggle("doThreshold");
	panel.addToggle("depthImg/diffImg");
	panel.addSlider("thresholdNear", 255, 0, 255);
	panel.addSlider("nearClipping", 700, 500, 4000);
	panel.addSlider("farClipping", 3000, 500, 4000);
	panel.addSlider("noteRange", 16, 7, 48, true);

    sender.setup("localhost", 50501);
    receiver.setup(12346);

	ofSetFrameRate(20);
	//int thing = 5;
	//ofLog(OF_LOG_NOTICE, "test address: " + ofToString(&thing));
}

//--------------------------------------------------------------
void testApp::update() {
	ofBackground(153);
	kinect.setDepthClipping( panel.getValueF("nearClipping"), panel.getValueF("farClipping") );
	kinect.update();
	box2d.update();
	if(!(ofGetFrameNum() % 2)){

		//get Pixel Data
		depthImg.setFromPixels(kinect.getDepthPixelsRef());
		depthImg.mirror(false, true);
		//Threshold on depth image
		if(panel.getValueB("doThreshold")) {
		    threshold(depthImg, panel.getValueF("thresholdNear"));  //invert to get one end of volume
		}
		absdiff(depthImg, previous, diffImg);
		diffImg.update();
		depthImg.update();
		//Find Contours on the image
		contourFinder.findContours(depthImg);
		tracker.track(contourFinder.getPolylines());
		kContours = tracker.getFollowers();

		//Copy the image to previous for differences
		copy(depthImg, previous);

		kinectVertices.clear();

		int verticesSize = 0;
		//sort the contours so the left most point is always first
		for (int i=0; i<kContours.size(); i++) {
		    //ofLog(OF_LOG_NOTICE, ofToString(kContours[i].getZeroIndex()));
		    vector <ofPoint> pts = kContours[i].getPoly().getVertices();
		    unsigned int zIndex = kContours[i].getZeroIndex();
		    std::rotate(pts.begin(), pts.begin()+zIndex, pts.end());

		    kinectVertices.insert(kinectVertices.end(), pts.begin(), pts.end());
		    verticesSize += pts.size();
		}

		int index = controlCircles.size();
		while(controlCircles.size() < verticesSize) {
		    ofxBox2dCircle circle;
		    circle.fixture.filter.categoryBits = 0x0001;
		    circle.fixture.filter.maskBits = 0x000E;
		    circle.setPhysics(10.0, 0.53, 0.1);
		    ofPoint pt = kinectVertices[index];
		    circle.setup(box2d.getWorld(),  pt.x, pt.y, CONTROL_CIRCLE_RADIUS);
		    controlCircles.push_back(circle);
		    polyJoints.push_back(createMouseJoint(pt.x, pt.y, circle));
		    index++;
		}

		for (int i=0; i<verticesSize; i++) {
		    //ofPoint cvPt = toOf(contourFinder.getCentroid(i));
		    controlCircles[i].fixture.filter.maskBits = 0x000E;
		    ofPoint knPt = kinectVertices[i];
		    //controlCircles[i].body->SetTransform(b2Vec2(b2dNum(knPt.x), b2dNum(knPt.y)), 0);
		    polyJoints[i]->SetTarget(b2Vec2(knPt.x/OFX_BOX2D_SCALE, knPt.y/OFX_BOX2D_SCALE));
		    //controlCircles[i].setPosition(toOf(contourFinder.getCentroid(i)));
		}

		for (int i=verticesSize; i<controlCircles.size(); i++){
		    controlCircles[i].body->SetTransform(b2Vec2(b2dNum(320), b2dNum(600)), 0);
		    polyJoints[i]->SetTarget(b2Vec2(320/OFX_BOX2D_SCALE, 600/OFX_BOX2D_SCALE));
		}

		for (int i=0; i<circles.size(); i++){
			    ofVec2f pos = circles[i].getPosition();
			    if(pos.y > 500){
                    circles[i].destroy();
                    //C++11 is some black magic
                    circles[i] = std::move(circles.back());
                    circles.pop_back();
                    ofxOscMessage m;
                    m.setAddress("/percussion");
                    m.addIntArg((int)ofMap(pos.x, 40, 640, 0, 3));
                    m.addFloatArg(ofMap(pos.x, 40,640, -1, 1));
                    sender.sendMessage(m);
			    }
		}

		while(receiver.hasWaitingMessages()){
		    // get the next message
		    ofxOscMessage m;
		    receiver.getNextMessage(&m);

		    // check for mouse moved message
		    if(m.getAddress() == "/drop"){
		        float r = (m.getArgType(0)==OFXOSC_TYPE_INT32)?m.getArgAsInt32(0):m.getArgAsInt64(0);
		        float pos = (m.getArgAsFloat(1));
		        ofxBox2dCircle circle;
		        circle.fixture.filter.categoryBits = 0x0002;
		        circle.fixture.filter.maskBits = 0x0001;
		        circle.setPhysics(3.0, 1.3, 0.1);
		        circle.setup(box2d.getWorld(), ofMap(pos, -1, 1, 40, 640), 10, r);
		        circles.push_back(circle);
		    }
		}
	}
}
//--------------------------------------------------------------
void testApp::draw() {
    ofPushMatrix();
    ofTranslate(-40,-40);
    ofSetColor(ofColor::fromHsb(127+sin(ofGetElapsedTimef()*2)*127, 255, 255));
    if(panel.getValueB("depthImg/diffImg")){
        depthImg.draw(0,0);
    } else {
         diffImg.draw(0,0);
    }
    ofSetColor(255, 255, 255);
    //ofDrawBitmapString(ofToString(mouseX) + ", " + ofToString(mouseY),100,100);

	for(int i=0; i<circles.size(); i++) {
		ofFill();
		ofSetHexColor(0xf1fff1);
		circles[i].draw();
	}

	for(int i=0; i<controlCircles.size(); i++) {
		ofNoFill();
		ofSetColor(255,0,0);
		controlCircles[i].draw();
	}

	// draw instructions
	ofSetColor(25);
	stringstream reportStream;
	reportStream << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
	<< ofToString(kinect.getMksAccel().y, 2) << " / "
	<< ofToString(kinect.getMksAccel().z, 2) << endl
    << "num blobs found " << contourFinder.size()
	<< ", fps: " << ofGetFrameRate() << endl
	<< "press c to close the connection and o to open it again, connection is: " << kinect.isConnected() << endl
	<< "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl
	<< "press 1-5 & 0 to change the led mode (mac/linux only)" << endl;
	ofDrawBitmapString(reportStream.str(),0,652);
	ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
	switch (key) {
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;

		case '<':
		case ',':
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;

		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;

		case '-':
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;

		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;

		case 'o':
			kinect.setCameraTiltAngle(angle); // go back to prev tilt
			kinect.open();
			break;

		case 'c':
			kinect.setCameraTiltAngle(0); // zero the tilt
			kinect.close();
			break;

		case '1':
			kinect.setLed(ofxKinect::LED_GREEN);
			break;

		case '2':
			kinect.setLed(ofxKinect::LED_YELLOW);
			break;

		case '3':
			kinect.setLed(ofxKinect::LED_RED);
			break;

		case '4':
			kinect.setLed(ofxKinect::LED_BLINK_GREEN);
			break;

		case '5':
			kinect.setLed(ofxKinect::LED_BLINK_YELLOW_RED);
			break;

		case '0':
			kinect.setLed(ofxKinect::LED_OFF);
			break;

        case '8':
            for(int i=0; i<circles.size(); i++){
                b2Body* bod = circles[i].body;
                b2Vec2 center = bod->GetWorldCenter();
                //bod->SetLinearVelocity(b2Vec2(bod->GetLinearVelocity().x, bod->GetLinearVelocity().y * -1));
                bod->ApplyLinearImpulse(b2Vec2(0, -30), center);
            }
            break;

		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;

		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;
	}

	if(key == 'q') {
		float r = ofRandom(10, 15);		// a random radius 4px - 20px
		ofxBox2dCircle circle;
		circle.fixture.filter.categoryBits = 0x0002;
		circle.fixture.filter.maskBits = 0x0001;
		circle.setPhysics(3.0, 0.53, 0.1);
		circle.setup(box2d.getWorld(), mouseX, mouseY, r);
		circles.push_back(circle);
	}
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    panel.setPosition(ofGetWidth()-320,0);
}

//--------------------------------------------------------------
void testApp::contactStart(ofxBox2dContactArgs &e) {
   /* if(e.b->GetType() == 1) {
            ofLog(OF_LOG_NOTICE, "Contact Begun");
            b2WorldManifold * mani = new b2WorldManifold();
            e.contact->GetWorldManifold(mani);
            b2Body* bodyA = e.a->GetBody();
            kinectPolyData * data = (kinectPolyData*) e.b->GetBody()->GetUserData();
            //bodyA->ApplyLinearImpulse(b2Vec2(mani->normal.x*30, mani->normal.y*30), bodyA->GetWorldCenter());
            bodyA->ApplyLinearImpulse(b2Vec2(data->velocity.x*30, data->velocity.y*30), bodyA->GetWorldCenter());
            delete mani;
	}
	else{
            b2WorldManifold * mani = new b2WorldManifold();
            e.contact->GetWorldManifold(mani);
            b2Body* bodyB = e.b->GetBody();
            kinectPolyData * data = (kinectPolyData*) e.a->GetBody()->GetUserData();
            bodyB->ApplyLinearImpulse(b2Vec2(data->velocity.x*.1, data->velocity.y*.1), bodyB->GetWorldCenter());
            delete mani;
	}*/
    //b2Body* bodyA = e.a->GetBody();
    //b2Body* bodyB = e.b->GetBody();

    b2Vec2 velB = e.b->GetBody()->GetLinearVelocity();
    b2Vec2 velA = e.a->GetBody()->GetLinearVelocity();
    float velTot = (abs(velA.x)+abs(velA.y)+abs(velB.x)+abs(velB.y))/4;
    b2Vec2 posA = e.a->GetBody()->GetPosition();
    int num = (int) ofMap(posA.y * OFX_BOX2D_SCALE, 0, 480, 0, panel.getValueI("noteRange"));
    float pos = ofMap(posA.x * OFX_BOX2D_SCALE, 0, 640, -1, 1);
    //ofLog(OF_LOG_NOTICE, "pos: " + num);
	ofxOscMessage m;
	m.setAddress("/note");
	m.addIntArg(num);
	m.addFloatArg(ofMap(velTot, 0, 7, 0.1, 1.0f));
	m.addFloatArg(pos);
	sender.sendMessage(m);
}

//--------------------------------------------------------------
void testApp::contactEnd(ofxBox2dContactArgs &e) {
	if(e.a->GetFilterData().categoryBits & e.b->GetFilterData().categoryBits) {
            //ofLog(OF_LOG_NOTICE, "Contact Over");
	}
}

b2MouseJoint* testApp::createMouseJoint(float x, float y, ofxBox2dCircle circle) {
    b2BodyDef bd;
    b2Body* mouseBody = box2d.getWorld()->CreateBody(&bd);
    b2MouseJointDef md;
    md.bodyA    = mouseBody;
    md.bodyB    = circle.body;
    md.target   = b2Vec2(x/OFX_BOX2D_SCALE, y/OFX_BOX2D_SCALE);
    md.maxForce = 150.0f * circle.body->GetMass();
    b2MouseJoint* mouseJoint  = (b2MouseJoint*)box2d.getWorld()->CreateJoint(&md);
}

bool operator<(ofPoint pt1, ofPoint pt2) {
    return (pt1.y < pt2.y)? true: false;
    return (pt1.x < pt2.x);
}

bool operator<(cv::Point2f pt1, cv::Point2f pt2) {
    return  3*pt2.x + pt2.y<3*pt1.x +pt1.y;
}
