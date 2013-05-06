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
*   [ofxControlPanel](https://github.com/kylemcdonald/ofxControlPanel)  
*   [ofxCv](https://github.com/kylemcdonald/ofxCv)  
*   [ofxKinect](https://github.com/ofTheo/ofxKinect)  
*   [ofxBox2d](https://github.com/vanderlin/ofxBox2d)  
and core addons:  
*   ofxOsc  
*   ofxXmlSettings  
*   ofxOpenCv  
  
background:
-----------
My final project is called the well-tempered-clavicle, an installation which uses a Kinect to gather information about physical objects, then track their movement to give them presence in a simulated, 2D-physical environment. In parallel, a music server provides both a background drone, as well as dynamically generated music created by collisions and destruction of circles. I’ll briefly go over my process up to this point, and then design strategy and overall conclusion of my work.
This was not my original plan. My first idea was to use computer vision to make a complex, sketchable user interface on paper, based off a project by Billy Keyes. However, it’s use of a projector which I was adamant about not using was actually integral, so seeing as how my idea couldn’t be readily implemented from scratch in the time window, I pivoted to making a novel interface, and maybe focus on trying something I had never done before, like music, instead of something that was already in my wheelhouse, graphics and computer vision. 
	My design strategy was built to be as modular as possible. The fact that OSC exists is invaluable in projects, and made integration a dream. My heavyweight physics and computer vision was done in openFrameworks, and the music was done in Overtone, a SuperCollider front-end for Clojure. Clojure starts a simple drone in C mixolydian, and the the openFrameworks app responds by requesting different musical actions for collisions and so forth. Clojure controls all the music logic, so it is very feasible to change the musical structure on the fly. I think this structure is very powerful for doing any music related applications, and I plan on utilizing this structure for the future.
Overall, I really enjoyed this experience. Bringing a project from concept to realization under time constraints really does present an enjoyable challenge. It really forced me to think about efficiency. I felt that it may have led me to hack some things together in my code, so I should probably revisit it before I forget intimately how it works. This was also my first experience with computer music, so it was really eye-opening in that respect.

