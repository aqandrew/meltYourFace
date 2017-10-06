#pragma once

/*
 Main methods adapted from meshFromCamera example.
 */

#include "ofMain.h"
#include "ofxEasyFft.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

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
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void plot(vector<float> & buffer, float scale);
        void setAudioSource(bool& _useMicrophone);

        ofCamera cam; // add mouse controls for camera movement
        float extrusionAmount;
        ofVboMesh mainMesh;
        ofVideoGrabber vidGrabber;

        ofxEasyFft fft;
        ofSoundStream audioInput;
        void audioIn(float * input, int bufferSize, int nChannels);
        int samplesPerBuffer;
        vector<float> left;
        vector<float> right;
        float smoothedVol;

        void setupGui();
        ofxPanel panel;
        ofParameter<bool> toggleGuiDraw;
        ofParameter<bool> doFullScreen;
        ofParameter<bool> useMicrophone;
        void setFullScreen(bool& _value) { ofSetFullscreen(_value); }
        ofParameter<bool> showPlot;
        int plotPadding;
		
};
