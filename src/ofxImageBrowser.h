#pragma once

#include "ofMain.h"
#include "map"

class ofxThreadedImageLoader_;

enum ofxImageBrowserOperationType
{
	BROWSER_NONE=0,
	BROWSER_OVER_IMAGE,	
	BROWSER_CLICK_IMAGE,
	BROWSER_DRAGGING_IMAGE,
	BROWSER_RELEASE_IMAGE
};
class ofxImageBrowserEventArgs :
	public ofEventArgs 
{
public:
	ofxImageBrowserEventArgs(
		ofxImageBrowserOperationType Op,
		ofFile ImgF,
		ofRectangle thumbRect,
		ofImage* ptrThumb,
		ofVec2f pos);
	ofxImageBrowserEventArgs();	
public:
	ofxImageBrowserOperationType Type;
	ofFile	ImgFile;	
	ofRectangle ThumbRect;
	ofImage* pThumb;
	ofVec2f Pos;	
};

class ofxImageBrowser
{
	// -------------------- nest definition -------------------------------------//
public:
	class opRecord
	{
	public:
		opRecord(int ccapacity = 5);
		void addPoint(ofPoint p);
		ofVec3f getVelocity();
		void clear();
		int size();
	private:
		int capacity;
		deque<ofPoint> Ps;
		deque<float> Ts;		
	};
	struct DispTargetImg
	{
	public:
		ofImage* pImg;
		ofVec3f Scale;
		ofVec3f Pos;
		float NDistToUserPos;
		ofColor BoundColor;
	public:
		DispTargetImg(
			ofImage* ppImg,
			ofVec3f scl,
			ofVec3f pos,
			float ndist,
			ofColor C = ofColor::white);
	};

	// -------------------- event -----------------------------------------//
public:
	// to respond to the action of this object, addListener to this event
	ofEvent <ofxImageBrowserEventArgs> ImgBrowseEvent;
	
	// -------------------- public methods -------------------------------------//
public:
	// init: need called before operation
	ofxImageBrowser(
		string DirPath = "", 
		string ext = "jpg",
		ofRectangle displayRect = 
			ofRectangle(0,0,ofGetWidth(),ofGetHeight()));
	~ofxImageBrowser();
	void setDirectoryAndExtenstion(
		string DirPath, string Ext);
	void setDispRect( ofRectangle displayRect );

	// set/get parameters
	void setOperable(bool bOperable);
	bool isOperable();
	bool inside( int x, int y ) const;

	void setThumbSize(float s);
	void setThumbScale(float s);
	void setNormalizedControlRadius(float r);
	void setControlScaleAmt(float a);
	void setBackgroundColor(ofColor C);
	
	float getThumbSize();	
	float getThumbScale();
	float getNormalizedControlRadius();
	float getControlScaleAmt();
	ofColor getBackgroundColor();
	
	// operation: need to be called by a host
	void opUpdate();
	void opDraw();	
	void opMove(float x, float y, float z=0, int id=0);
	void opPress(float x, float y, float z=0, int id=0);
	void opDrag(float x,float y,float z=0, int id=0);
	void opRelease(float x, float y, float z=0, int id=0);

	// -------------------- private methods -------------------------------------//
private:
	void resetThumbRect();
	void clearThumbFbo();
	void resetThumbFbo();
	void drawToThumbFbo();
	void generateDisplayList();
	void drawDisplayListOnThumbFbo();

	void drawDispTarget( DispTargetImg &DT );

	


	void ThumbSizeChanged(float &s);
	ofRectangle getScrolledThumbRect();
	void notifyBrowseEvent( float x, float y, ofxImageBrowserOperationType ET);

private:
	// debug fcns:
	void debugCout( string s );
	void resetMovement();
	void pushMSV();
	void popMSV();
	
	// -------------------- props -------------------------------------------//
private:	
	// configuration
	ofDirectory D;		
	ofParameter<float> ThumbSize;
	ofParameter<float> ThumbScale;
	ofParameter<float> UserControlRadius;
	ofParameter<float> UserControlAmt;
	ofRectangle DispRect;
	ofParameter<bool> ScrollableHorizontal;
	ofParameter<bool> ScrollableVertical;
	ofParameter<float> ScrollVelDampPerSec;
	ofParameter<ofVec3f> ScrollVelCoef;
	ofParameter<ofColor> BgColor;

	// state
	float tThumbSize;
	ofVec3f PressPos;
	ofVec3f UserPos;
	ofRectangle ThumbRect;
	ofFbo ThumbFbo;
	int thumbCols, thumbRows;
	map<ofFile, ofImage> ImgList;
	multimap<float, DispTargetImg> ImgDispList;
	float pCenterDispTgtImgKey;
	bool bRunning;
		
	ofVec3f Trans;
	ofVec3f Vel;	
	ofVec3f PointPrev;
	bool bPressed;
	float TLastUpdate;
	
	opRecord Trace;


	// helper	
	ofPtr<ofxThreadedImageLoader_> pImgLoader;

	

};








