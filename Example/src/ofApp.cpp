#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	I.allocate(100,100,OF_IMAGE_COLOR);
	Browser.setDirectoryAndExtenstion("Images\\","png");
	ofRectangle BrowserRect(0,0,ofGetWidth()/2,ofGetHeight());
	Browser.setDispRect(BrowserRect);
	Browser.setOperable(true);
	Browser.setThumbSize(120);
	ofAddListener(Browser.ImgBrowseEvent,this,&ofApp::imageBrowseEvent);
}

//--------------------------------------------------------------
void ofApp::update(){
	Browser.opUpdate();
}

//--------------------------------------------------------------
void ofApp::draw(){
	Browser.opDraw();

	ofRectangle RI(ofGetWidth()/2,0,ofGetWidth()/2,ofGetHeight());

	RI.scaleFromCenter(0.99f);
	I.draw(RI.x,RI.y,256,256);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	
	Browser.opMove(x,y);
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	if(button==0)
	{
		Browser.opDrag(x,y);
	}	
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if(button==0)
	{
		Browser.opPress(x,y);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	if(button==0)
	{
		Browser.opRelease(x,y);
	}
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

void ofApp::imageBrowseEvent( ofxImageBrowserEventArgs &e )
{
	if(e.Type == BROWSER_CLICK_IMAGE)
	{
		I.loadImage(e.ImgFile);
		cout << "Load image: " << e.ImgFile.getFileName() << endl;
	}	
	
}
