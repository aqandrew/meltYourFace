#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    smoothedVol = 0.0;
    samplesPerBuffer = 128;
    fft.setup(2048);
    audioInput.setup(this, 0, 2, 44100, samplesPerBuffer, 4);
    left.assign(samplesPerBuffer, 0.0);
    right.assign(samplesPerBuffer, 0.0);

    ofSetVerticalSync(true);
    ofSetFrameRate(60);
    ofBackground(66,66,66);

    //initialize the video grabber
    vidGrabber.setVerbose(true);
    vidGrabber.setup(320,240);

    //store the width and height for convenience
    int width = vidGrabber.getWidth();
    int height = vidGrabber.getHeight();

    //add one vertex to the mesh for each pixel
    for (int y = 0; y < height; y++){
        for (int x = 0; x<width; x++){
            mainMesh.addVertex(ofPoint(x,y,0));    // mesh index = x + y*width
            // this replicates the pixel array within the camera bitmap...
            mainMesh.addColor(ofFloatColor(0,0,0));  // placeholder for colour data, we'll get this from the camera
        }
    }

    for (int y = 0; y<height-1; y++){
        for (int x=0; x<width-1; x++){
            mainMesh.addIndex(x+y*width);                // 0
            mainMesh.addIndex((x+1)+y*width);            // 1
            mainMesh.addIndex(x+(y+1)*width);            // 10

            mainMesh.addIndex((x+1)+y*width);            // 1
            mainMesh.addIndex((x+1)+(y+1)*width);        // 11
            mainMesh.addIndex(x+(y+1)*width);            // 10
        }
    }

    //this is an annoying thing that is used to flip the camera
    cam.setScale(1,-1,1);


    //this determines how much we push the meshes out
    extrusionAmount = 70.0;

    setupGui();

    plotPadding = 30;
}

void ofApp::setupGui(){
    panel.setup("settings");
    panel.setDefaultBackgroundColor(ofColor(0, 0, 0, 127));
    panel.setDefaultFillColor(ofColor(160, 160, 160, 160));

    panel.add(doFullScreen.set("fullscreen (F)", false));
    doFullScreen.addListener(this, &ofApp::setFullScreen);
    panel.add(toggleGuiDraw.set("show GUI (G)", true));
    panel.add(showPlot.set("show plot (P)", true));
    panel.add(useMicrophone.set("use microphone (M)", true));
    useMicrophone.addListener(this, &ofApp::setAudioSource);
}

//--------------------------------------------------------------
void ofApp::update(){
    fft.update();
    float value = fft.getBins()[50]; // this will be the amplitude at C#5
//    float value = smoothedVol; // TODO figure out why audioIn isn't getting called.
    // Maybe something to do with multiple input channels?

    //grab a new frame
    vidGrabber.update();

    //update the mesh if we have a new frame
    if (vidGrabber.isFrameNew()){
        for (int i=0; i<vidGrabber.getWidth()*vidGrabber.getHeight(); i++){

            ofFloatColor sampleColor(vidGrabber.getPixels()[i*3]/255.f,                // r
                                     vidGrabber.getPixels()[i*3+1]/255.f,            // g
                                     vidGrabber.getPixels()[i*3+2]/255.f);            // b

            //now we get the vertex at this position
            ofVec3f tmpVec = mainMesh.getVertex(i);

            //melt a little if the sound is loud enough
            float threshold = (useMicrophone.get()) ? 0.06 : 0.01;
            if (value > threshold) {
                int yInitial = tmpVec.y;
                tmpVec.y += (sampleColor.getBrightness() * value * 6) / 3;
                tmpVec.y = (int)tmpVec.y % (int)vidGrabber.getHeight(); // make the bottom pixels jump to the top
                tmpVec.z += (tmpVec.y - yInitial) / 5;
//                tmpVec.y += (sampleColor.getBrightness()) / 8;
            }
            mainMesh.setVertex(i, tmpVec);

            mainMesh.setColor(i, sampleColor);
        }
    }

    //vertically center the camera
    float rotateAmount = 0;

    //move the camera around the mesh
    ofVec3f camDirection(0,0,1);
    ofVec3f centre(vidGrabber.getWidth()/2.f,vidGrabber.getHeight()/2.f, 255/2.f);
    ofVec3f camDirectionRotated = camDirection.getRotated(rotateAmount, ofVec3f(1,0,0));
    ofVec3f camPosition = centre + camDirectionRotated * extrusionAmount;

    cam.setPosition(camPosition);
    cam.lookAt(centre);
}

//--------------------------------------------------------------
void ofApp::draw(){
    //we have to disable depth testing to draw the video frame
    ofDisableDepthTest();
    //    vidGrabber.draw(20,20);

    //but we want to enable it to show the mesh
    ofEnableDepthTest();
    cam.begin();

    //You can either draw the mesh or the wireframe
    // mainMesh.drawWireframe();
    mainMesh.drawFaces();
    cam.end();

    //draw framerate for fun
//    ofSetColor(255);
//    string msg = "fps: " + ofToString(ofGetFrameRate(), 2);
//    ofDrawBitmapString(msg, 10, 20);

    if (showPlot) {
        int plotScale = 128;
        //plot FFT for debugging purposes
        ofPushMatrix();
        ofTranslate(16, ofGetHeight() - plotScale - plotPadding);
        ofSetColor(255);
        ofDrawBitmapString("Frequency Domain", 0, 0);
        plot(fft.getBins(), plotScale);
        ofPopMatrix();
    }

    if (toggleGuiDraw.get()) {
        ofDisableDepthTest();
        panel.draw();
    }
}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){
    cout << "audioIn called!" << endl;
    // see audioInputExample
    float curVol = 0.0;

    // samples are "interleaved"
    int numCounted = 0;

    //lets go through each sample and calculate the root mean square which is a rough way to calculate volume
    for (int i = 0; i < bufferSize; i++){
        left[i]        = input[i*2]*0.5;
        right[i]    = input[i*2+1]*0.5;

        curVol += left[i] * left[i];
        curVol += right[i] * right[i];
        numCounted+=2;
    }

    //this is how we get the mean of rms :)
    curVol /= (float)numCounted;

    // this is how we get the root of rms :)
    curVol = sqrt( curVol );

    smoothedVol *= 0.93;
    smoothedVol += 0.07 * curVol;

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key) {
        case 'f':
            doFullScreen.set(!doFullScreen.get());
            break;
        case 'g':
            toggleGuiDraw.set(!toggleGuiDraw.get());
            break;
        case 'p':
            showPlot.set(!showPlot.get());
            break;
        case 'm':
            useMicrophone.set(!useMicrophone.get());
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::plot(vector<float> & buffer, float scale) {
    // Note: plot is drawn relative to label, drawn in draw()
    ofNoFill();
    int n = MIN(1024, buffer.size());
    ofDrawRectangle(0, plotPadding / 2, n, scale);
    ofPushMatrix();
    ofTranslate(0, scale + plotPadding / 2);
    ofScale(1, -scale);
    ofBeginShape();
    int fractionFactor = 1; // sometimes we only care about lower-frequency sounds; scale them up
    for (int i = 0; i < n / fractionFactor; i++) {
        for (int f = 0; f < fractionFactor; f++) {
            ofVertex(i * fractionFactor + f, buffer[i]);
        }
    }
    ofEndShape();
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::setAudioSource(bool& _useMicrophone) {
    if (_useMicrophone) {
        fft.stream.setDeviceID(0); // microphone
        audioInput.setDeviceID(0);
    } else {
        fft.stream.setDeviceID(5); // Loopback audio
        audioInput.setDeviceID(5);
    }

    fft.setup(2048);
    audioInput.setup(this, 0, 2, 44100, samplesPerBuffer, 4);

}
