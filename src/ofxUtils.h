#pragma once
#ifndef OFXUTILS_H
#define OFXUTILS_H

#include <string>
#include <vector>
#include "ofMain.h"

#define ITERATOR2POINTER(it) (&*it)

using namespace std;
namespace fp
{
	std::string randomString(int len);
	float getCircleDiameterFromArea(float A);
	void fillFboWithChar(
		ofFbo& fbo,									// the fbo to be filled
		char c,										// the character
		float relativeSize		= 0.9f,				// the relative size of the character versus the fbo
		ofColor backgroundColor = ofColor::white,	// the background color
		ofColor charColor		= ofColor::black);	// the character color
	void fboLoadImageFile(ofFbo& fbo, string File);	// load image into fbo
	ofImage genSoftCircleImage(int diameter, float softness);	// generate a soft edged white circle
	void fillFboWithSoftCircle(ofFbo& fbo, float softness);		// fill fbo with a soft edged white circle
	void fillFboAlphaWithSoftCircle(ofFbo& fbo, float softness);	// fill fbo's alpha with a soft edged circle
	void fillFboWithImage(ofFbo& fbo,
                          ofImage& img,
                          bool FillAlpha=true,
                          float EmptyBorder=0.0f);				// fill the fbo with an image
	void fillFboWithTex(ofFbo& fbo, ofTexture& tex, bool FillAlpha=true);
	void fillFboWithColor(ofFbo& fbo,ofColor c,bool FillAlpha = false);
	std::string random_string( size_t length );		// generate random string
	
	ofVec4f nomalizeLocation( float WD, float HT, ofVec4f Loc );
	ofVec4f denomalizeLocation( float WD, float HT, ofVec4f NLoc );
	ofColor getFboColorAtPoint(ofFbo* fbo, ofPoint pos);
    char randchar();

	ofRectangle getGridSubRect(				// get the rectangle of a sub grid
		ofRectangle R,						// the input rectangle of the grid
		int GridSizeM, int GridSizeN,		// input grid size
		int i,int j);						// the sub-grid indices
	ofRectangle getGridSubRect(				// get the rectangle of a sub grid
		ofRectangle R,						// the input rectangle of the grid
		int GridSizeM, int GridSizeN,		// input grid size
		int id);							// the sub-grid indices
	
	void clearTempDir();					// clear temporary directory
	void createTempDir();					// create temporary directory


	// from camera screen coordinate to z=0 plane coordinate
	void posCameraScreenToPosXYPlane(
		ofCamera& pCam,				// the camera through which the 3D scene is displayed on the screen
		const ofVec2f& PosScreen,	// the xy Coordinates on the displayed screen of the camera
		ofVec3f& PosXY);			// the returned value: the xy coordinates in 3D space, which is a point on z=0 plane 


	// get the intersection of a vector and a plane
	bool intersectPlane(const ofVec3f &n, const ofVec3f &p0, const ofVec3f& l0, const ofVec3f &l, float &d);

	// get the intersection of a vector and a circle disk
	bool intersectCircleDisk(const ofVec3f &n, const ofVec3f &p0, const float &radius, const ofVec3f &l0, const ofVec3f &l);
	

	// mipmap generation
	void loadMipMapTexture( ofTexture& inTex, string imgPath, float inAnisotropy );	

}

#endif

