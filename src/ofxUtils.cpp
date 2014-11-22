#include "ofxUtils.h"

#include <time.h>
#include <algorithm>
//#include "spline.hpp"

std::string fp::randomString(int len)
{
	srand(clock());
	std::string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	int pos;
	while(str.size() != len) {
		pos = ((rand() % (str.size() - 1)));
		str.erase (pos, 1);
	}
	return str;
}

float fp::getCircleDiameterFromArea( float A )
{
	return 2.0f*sqrt(A/3.1415926535897384626f);
}

void fp::fillFboWithChar( 
	ofFbo& fbo, /* the fbo to be filled */ 
	char c, /* the character */ 
	float relativeSize /*= 0.9f*/,					/* the relative size of the character versus the fbo */ 
	ofColor backgroundColor /*= ofColor::white*/,	/* the background color */ 
	ofColor charColor /*= ofColor::black*/ )
{
	float wd,ht;
	wd = fbo.getWidth();
	ht = fbo.getHeight();

	relativeSize = (relativeSize>1.0f)?1.0f:relativeSize;
	relativeSize = (relativeSize<0.0f)?0.0f:relativeSize;
	float x,y;
	x = (1.0f-relativeSize)*wd/2.0f;
	y = (1.0f-relativeSize)*ht/2.0f;

	ofTrueTypeFont F;
	float Min = min(wd,ht);
	float FontSize = Min*relativeSize;
	F.loadFont("Fonts/vag.ttf",FontSize,true,false,true);
	F.setLineHeight(FontSize);
	string str;
	str += c;

	// draw the string into a temp fbo
	float W = F.stringWidth(str);
	float H = F.stringHeight(str);
	ofFbo tempFbo;
	tempFbo.allocate(W,H);
	tempFbo.begin();
	{
		ofSetBackgroundColor(ofColor(0.0f,0.0f,0.0f,1.0f));
		ofSetColor(charColor);
		F.drawString(str,0,0);
	}
	tempFbo.end();

	// draw the temp fbo into the target fbo
	fbo.begin();
	{		
		ofSetBackgroundColor(backgroundColor);
		ofSetColor(charColor);		
		tempFbo.draw(x,y,W,H);
	}
	fbo.end();	
	ofSetBackgroundColor(ofColor::white);
	ofSetColor(ofColor::white);


}

void fp::fboLoadImageFile( ofFbo& fbo, string File )
{
	ofImage img;
	img.loadImage(File);

	if(!fbo.isAllocated())
	{
		fbo.allocate(img.getWidth(),img.getHeight());
	}

	float wd,ht;
	wd = fbo.getWidth();
	ht = fbo.getHeight();
	
	fbo.begin();
	{
		ofPushStyle();
		ofSetColor(ofColor::white);		
		//ofSetBackgroundColor(ofColor::white);
		//glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ZERO);

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColorMask(true,true,true,false);
		img.draw(0,0,wd,ht);

		glBlendFunc(GL_ONE,GL_ZERO);
		glColorMask(false,false,false,true);
		img.draw(0,0,wd,ht);

		glColorMask(true,true,true,true);

		ofPopStyle();
	}
	fbo.end();
}

char fp::randchar(){

		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[ rand() % max_index ];
}
// random string generation
std::string fp::random_string( size_t length )
{
	std::string str(length,0);
	std::generate_n( str.begin(), length, randchar );
	return str;
}

ofVec4f fp::nomalizeLocation( float WD, float HT, ofVec4f Loc )
{
	float SquareSize = sqrt(WD*HT);
	ofVec4f NLoc = Loc/SquareSize;
	return NLoc;
}
ofVec4f fp::denomalizeLocation( float WD, float HT, ofVec4f NLoc )
{
	float SquareSize = sqrt(WD*HT);
	ofVec4f NLocLocal = NLoc*SquareSize;
	return NLocLocal;
}

ofImage fp::genSoftCircleImage(int diameter, float softness)
{
	ofImage I;
	I.allocate(diameter,diameter,OF_IMAGE_COLOR_ALPHA);

	softness = softness>1.0f?1.0f:softness;
	softness = softness<0.0f?0.0f:softness;

	float xc,yc;
	xc = diameter/2.0f;
	yc = xc;

	float radius = diameter/2.0f;
	float softStart = (1.0f-softness)*radius;
	float softLength = softness*radius-1;


	for(int i =0;i<diameter;i++)
	{
		for(int j=0;j<diameter;j++)
		{
			float x = i-xc;
			float y = j-yc;
			float r = ofDist(x,y,0,0);
			if (r>softStart)
			{
				float opaqueness = 1.0-(r-softStart)/softLength;
				opaqueness = ofClamp(opaqueness,0.0f,1.0f);					
				int a = 255*opaqueness;
				ofColor c(255,255,255,a);
				I.setColor(i,j,c);

				/*if(a>250)
				{
					cout << c << endl;
				}*/
			}
			else
			{
				I.setColor(i,j,ofColor(255,255,255,255));
			}
		}
	}

	return I;
}
void fp::fillFboWithSoftCircle(ofFbo& fbo, float softness)
{
	float wd,ht;
	wd = fbo.getWidth();
	ht = fbo.getHeight();

	// debug
	//cout << "fbo WD: " << wd << " HT: "<< ht << endl;

	float diameter = max(wd,ht);

	ofImage  I;
    I.allocate((int)diameter,(int)diameter,OF_IMAGE_COLOR_ALPHA);
    I = genSoftCircleImage((int)diameter, softness);
		
    //I.loadImage("Brushes/brush.png");
	fbo.begin();
	{		
		ofEnableAlphaBlending();
		ofSetColor(255,255,255,255);		
		//glBlendFuncSeparate(GL_ONE,GL_ZERO,GL_ONE,GL_ZERO);
		glColorMask(true,true,true,true);
		glBlendFunc(GL_ONE,GL_ZERO);
		I.draw(0,0,0,wd,ht);		
	}
	fbo.end();

	
}

void fp::fillFboAlphaWithSoftCircle(ofFbo& fbo, float softness)
{
	float wd,ht;
	wd = fbo.getWidth();
	ht = fbo.getHeight();

	float diameter = max(wd,ht);
	ofImage  I;
	I = genSoftCircleImage((int)diameter, softness);

	fbo.begin();
	{		
		ofEnableAlphaBlending();
		ofSetColor(255,255,255,255);		
		//glBlendFuncSeparate(GL_ONE,GL_ZERO,GL_ONE,GL_ZERO);
		glBlendFunc(GL_ONE,GL_ZERO);
		glColorMask(false,false,false,true);
		I.draw(0,0,0,wd,ht);		
		glColorMask(true,true,true,true);
	}
	fbo.end();
}

void fp::fillFboWithImage( ofFbo& fbo, ofImage& img,
                          bool FillAlpha, float EmptyBorder)
{
	// check fbo
	if (!fbo.isAllocated())
	{
		fbo.allocate(img.getWidth(),img.getHeight(),GL_RGBA);		
	}

	// get the width and height of the fbo
	float wd,ht;
	wd = fbo.getWidth();
	ht = fbo.getHeight();

	// fill the fbo
	//ofEnableAlphaBlending();
	ofSetColor(ofColor::white);
	glBlendFunc(GL_ONE,GL_ZERO);
	glColorMask(true,true,true,FillAlpha);
		
	// memory bug!!!!!
	fbo.begin();
	{
        ofClear(0,0,0,0);
        float edgeh,edgev;
        edgeh = wd*EmptyBorder;
        edgev=  ht*EmptyBorder;
        
		img.draw(edgeh,edgev,wd-2.0f*edgeh,ht-2.0f*edgev);
	}
	fbo.end();	
	glColorMask(true,true,true,true);
}

void fp::fillFboWithTex( ofFbo& fbo, ofTexture& tex, bool FillAlpha/*=true*/ )
{
	// get the width and height of the fbo
	float wd,ht;
	wd = fbo.getWidth();
	ht = fbo.getHeight();

	// fill the fbo
	ofEnableAlphaBlending();
	ofSetColor(ofColor::white);
	/*if (FillAlpha)
	{
		glBlendFuncSeparate(GL_ONE,GL_ZERO,GL_ONE,GL_ZERO);
	}
	else
	{
		glBlendFuncSeparate(GL_ONE,GL_ZERO,GL_ZERO,GL_ONE);
	}*/
	glBlendFunc(GL_ONE,GL_ZERO);
	if (!FillAlpha)
	{
		glColorMask(true,true,true,false);
	}
	else
	{
		glColorMask(true,true,true,true);
	}

	fbo.begin();
	{
		tex.draw(0,0,wd,ht);
	}
	fbo.end();	
	glColorMask(true,true,true,true);
}

ofColor fp::getFboColorAtPoint(ofFbo* fbo, ofPoint pos)
{
	ofPixels px;
	ofTexture tx = fbo->getTextureReference();
	GLbyte pixels[4];
	fbo->begin();
		glReadPixels(pos.x,pos.y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,(GLvoid*)pixels);
	fbo->end();

	ofColor c;
	c.r = pixels[0];
	c.g = pixels[1];
	c.b = pixels[2];
	c.a = pixels[3];
	return c;
}

void fp::fillFboWithColor( ofFbo& fbo,ofColor c, bool FillAlpha )
{
	fbo.begin();
	{
		ofEnableAlphaBlending();
		ofSetColor(c);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		

		if (!FillAlpha)
		{
			//glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ZERO,GL_ONE);
			glColorMask(true,true,true,false);
		}
		else
		{
			//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glColorMask(true,true,true,true);
		}
		ofRect(0,0,fbo.getWidth(),fbo.getHeight());
		glColorMask(true,true,true,true);
	}
	fbo.end();

}

ofRectangle fp::getGridSubRect( /* get the rectangle of a sub grid */ 
	ofRectangle R, /* the input rectangle of the grid */ 
	int GridSizeM, int GridSizeN, /* input grid size */ 
	int i,int j )
{
	ofRectangle R2(0,0,0,0);
	
	// check indices
	if(GridSizeM<1||GridSizeN<1)
	{
		return R2;
	}
	if(i<0||j<0||(i>GridSizeM-1)||(j>GridSizeN-1))
	{
		return R2;
	}	

	// get unit length x and y
	float X,Y,WD,HT;
	X = R.getMinX();
	Y = R.getMinY();
	WD = R.getWidth();
	HT = R.getHeight();
	float GSizeX = WD/GridSizeM;
	float GSizeY = HT/GridSizeN;

	// get the sub grid position
	float x,y,wd,ht;
	x = X + i*GSizeX;
	y = Y + j*GSizeY;
	wd = GSizeX;
	ht = GSizeY;

	R2.setPosition(x,y);
	R2.setWidth(wd);
	R2.setHeight(ht);
	
	return R2;
}

ofRectangle fp::getGridSubRect( /* get the rectangle of a sub grid */ 
	ofRectangle R, /* the input rectangle of the grid */ 
	int GridSizeM, int GridSizeN, /* input grid size */ 
	int id )
{
	ofRectangle R2(0,0,0,0);
	if(id<0)
	{
		return R2;
	}
	int i,j;

	i = id%GridSizeM;
	j = (id-i)/GridSizeM;

	
	R2 = fp::getGridSubRect(R,GridSizeM,GridSizeN,i,j);
	return R2;
}

void fp::clearTempDir()
{
	string SDir = "Temp";
	ofDirectory D(SDir);
	if(D.exists())
	{
		D.removeDirectory(SDir,true,true);
	}
	D.createDirectory(SDir);
}

void fp::createTempDir()
{
	string SDir = "Temp";
	ofDirectory D(SDir);
	if(!D.exists())
	{
		D.createDirectory(SDir);
	}	
}

// get the intersection of a vector and a plane
bool fp::intersectPlane(const ofVec3f &n, const ofVec3f &p0, const ofVec3f& l0, const ofVec3f &l, float &d)
{
	// assuming vectors are all normalized
	float denom = n.dot(l);
	//float denom = dot(n, l);
	if (abs(denom) > 1e-6) 
	{
		ofVec3f p0l0 = p0 - l0;
		//d = dot(p0l0, n) / denom; 
		d = p0l0.dot(n)/denom;
		return true;
	}
	return false;
}

// get the intersection of a vector and a circle disk
bool fp::intersectCircleDisk(const ofVec3f &n, const ofVec3f &p0, const float &radius, const ofVec3f &l0, const ofVec3f &l)
{
	float d = 0;
	if (fp::intersectPlane(n, p0, l0, l, d)) {
		ofVec3f p = l0 + l * d;
		ofVec3f v = p - p0;
		float d2 = v.dot(v);
		return (sqrtf(d2) <= radius);
		// or you can use the following optimisation (and precompute radius^2)
		// return d2 <= radius2; // where radius2 = radius * radius
	}
	return false;
}

void fp::posCameraScreenToPosXYPlane( ofCamera& pCam, /* the camera through which the 3D scene is displayed on the screen */ 
									 const ofVec2f& PosScreen, /* the xy Coordinates on the displayed screen of the camera */ 
									 ofVec3f& PosXY )
{
	ofVec3f V0(0,0,0);
	ofVec3f V0S = pCam.worldToScreen(V0);		

	float VMSY=PosScreen.y;
	if(pCam.isVFlipped())
	{
		VMSY = ofGetWidth()-PosScreen.y;
	}
	ofVec3f VMS(PosScreen.x,ofGetHeight()-PosScreen.y,V0S.z);
	ofVec3f VMW = pCam.screenToWorld(VMS);
	ofVec3f CamPos = pCam.getPosition();
	ofVec3f VCam2MW = VMW-CamPos;  // the vector from camera position to mouse position on the paint surface(the plane: z=0)
	float d;
	bool bIntersect = fp::intersectPlane(ofVec3f(0,0,1),ofVec3f(1,1,0),CamPos,VCam2MW,d);
	ofVec3f InterPos=CamPos;
	if(bIntersect)
	{
		InterPos += VCam2MW*d;
	}		
	float u,v; // the position on the image plane
	u = InterPos.x;
	v = InterPos.y;
	
	PosXY.x = u;
	PosXY.y = v;
	PosXY.z = 0.0f;
}

void fp::loadMipMapTexture( ofTexture& inTex, string imgPath, float inAnisotropy ) {
	ofPixels pix;
	bool loaded = ofLoadImage(pix, imgPath );

	inTex.allocate( pix.getWidth(), pix.getHeight(), ofGetGlInternalFormat(pix) );

	ofTextureData& texData = inTex.texData;

	if (texData.textureTarget == GL_TEXTURE_RECTANGLE_ARB){
		texData.tex_t = pix.getWidth();
		texData.tex_u = pix.getHeight();
	} else {
		texData.tex_t = (float)(pix.getWidth()) / (float)texData.tex_w;
		texData.tex_u = (float)(pix.getHeight()) / (float)texData.tex_h;
	}

	ofSetPixelStorei(pix.getWidth(),pix.getBytesPerChannel(),pix.getNumChannels());

	glGenTextures(1, &texData.textureID);
	glBindTexture( texData.textureTarget, texData.textureID);
	glTexImage2D( texData.textureTarget, 0, texData.glTypeInternal, pix.getWidth(), pix.getHeight(), 0, ofGetGlFormat(pix), ofGetGlType(pix), pix.getPixels());
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(  texData.textureTarget);
	glGenerateMipmap(texData.textureTarget);
	glTexParameteri( texData.textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri( texData.textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// 1 means it does nothing //
	if( inAnisotropy >= 1 ) {
		glTexParameterf( texData.textureTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, (GLfloat)inAnisotropy);
	}

	glDisable( texData.textureTarget );
}


