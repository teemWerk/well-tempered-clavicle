well-tempered-clavicle
======================

Art installation made for IT for the Arts and Performance @ RPI. Powered by openFrameworks and clojure/Overtone.  
Play ambient music using the Kinect in a virtual 2D physical environment!  
Developed on x64 Linux Mint, tested to be functional on Windows. OSX shouldn't have a problem, nothing exotic.
The clojure code, however, won't work out of the box on win because it uses overtone.live, which does not seem
to be available on windows.

openFrameworks components:
--------------------------
clavicle makes extensive use of addons. Those addons needed which aren't in the core are:  
*[ofxControlPanel](https://github.com/kylemcdonald/ofxControlPanel)  
*[ofxCv](https://github.com/kylemcdonald/ofxCv)  
*[ofxKinect](https://github.com/ofTheo/ofxKinect)  
*[ofxBox2d](https://github.com/vanderlin/ofxBox2d)  
and core addons:  
*ofxOsc  
*ofxXmlSettings  
*ofxOpenCv
