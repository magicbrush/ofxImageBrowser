#include "ofxImageBrowser.h"
#include "ofxThreadedImageLoader_.h"
#include "ofxUtils.h"


ofxImageBrowser::opRecord::opRecord( int ccapacity /*= 7*/ )
{
	this->capacity = ccapacity;
}

void ofxImageBrowser::opRecord::addPoint( ofPoint p )
{
	float t = ofGetElapsedTimef();

	Ps.push_back(p);
	Ts.push_back(t);

	int Num = Ps.size();
	if(Num>capacity)
	{
		Ps.pop_front();
		Ts.pop_front();
	}
}

ofVec3f ofxImageBrowser::opRecord::getVelocity()
{	
	ofVec3f Vel(0,0,0);

	int Num = Ps.size();
	if(Num>3)
	{
		ofVec3f Trans = Ps.back()-Ps.front();
		float dt = Ts.back()-Ts.front();
		Vel = Trans/dt;
	}

	

	return Vel;
}

void ofxImageBrowser::opRecord::clear()
{
	Ps.clear();
	Ts.clear();
}

int ofxImageBrowser::opRecord::size()
{
	return Ps.size();
}


ofxImageBrowser::ofxImageBrowser( string DirPath /*= ""*/, 
								 string ext /*= "jpg"*/,
								 ofRectangle displayRect)
{	
	// configurable
	ThumbSize.setName("ThumbSize");
	ThumbSize.setMin(0.0f);
	ThumbSize.setMax(256.0f);
	ThumbSize = 80.0f;
	ThumbSize.addListener(this,&ofxImageBrowser::ThumbSizeChanged);
	
	ThumbScale.setName("ThumbScale");
	ThumbScale.setMin(0.0f);
	ThumbScale.setMax(1.0f);
	ThumbScale = 0.9f;
	
	UserControlRadius.setName("UserControlRadius");
	UserControlRadius.setMin(0.0001f);
	UserControlRadius.setMax(1.0f);
	UserControlRadius = 0.4f;
	
	UserControlAmt.setName("UserControlAmt");
	UserControlAmt.setMin(0.2f);
	UserControlAmt.setMax(5.0f);
	UserControlAmt = 1.8f;
	
	DispRect =displayRect;
	
	ScrollableHorizontal.setName("ScrollableHorizontal");
	ScrollableHorizontal = false;
	
	ScrollableVertical.setName("ScrollableVertical");
	ScrollableVertical = true;

	ScrollVelDampPerSec.setName("ScrollVelDampPerSec");
	ScrollVelDampPerSec.setMin(0.01f);
	ScrollVelDampPerSec.setMax(1.0f);
	ScrollVelDampPerSec = 0.5f;

	ScrollVelCoef.setName("ScrollVelCoef");
	ScrollVelCoef.setMax(ofVec3f(50.0f,50.0f,50.0f));
	ScrollVelCoef.setMin(ofVec3f(0.1f,0.1f,0.1f));
	ScrollVelCoef = ofVec3f(1.0f,1.0f,1.0f);

	BgColor = ofColor(255,255,255,0);

	// state:
	tThumbSize = ThumbSize;
	
	UserPos = ofVec3f(0,0,-100);
	PressPos = ofVec3f(0,0,-100);
	
	ThumbRect = displayRect;
	
	thumbCols = 0;
	thumbRows = 0;

	Trans = ofVec3f(0,0);
	Vel = ofVec3f(0,0);

	PointPrev = ofVec3f(0,0,0);

	bPressed = false;

	TLastUpdate = -1.0f;

	pCenterDispTgtImgKey = NULL;

	bRunning = true;

	// helper
	pImgLoader.reset(new ofxThreadedImageLoader_);
	pImgLoader->setResolutionLimits(ofVec2f(ThumbSize,ThumbSize));
	
	// init fcns
	setDirectoryAndExtenstion(DirPath,ext);

	setDispRect(displayRect);
}

ofxImageBrowser::~ofxImageBrowser()
{

}

void ofxImageBrowser::setDirectoryAndExtenstion( string DirPath, string Ext )
{
	D.reset();
	D.allowExt(Ext);
	D.listDir(DirPath);

	vector<ofFile> Fs = D.getFiles();
	int Num = Fs.size();
	for(int i=0;i<Num;i++)
	{
		map<ofFile, ofImage>::iterator it;
		it = ImgList.find(Fs[i]);
		if(it!=ImgList.end())
		{
			continue;
		}

		ofImage I;
		pair<ofFile, ofImage> Item(Fs[i],I);
		ImgList.insert(Item);

		pImgLoader->loadFromDisk(ImgList[Fs[i]],Fs[i].getAbsolutePath());
	}

	resetThumbRect();
	resetMovement();
}

void ofxImageBrowser::setDispRect( ofRectangle displayRect )
{
	DispRect = displayRect;

	resetThumbFbo();


	resetThumbRect();
	resetMovement();
}

void ofxImageBrowser::opUpdate()
{
	if(!isOperable()) {return;};

	if(TLastUpdate == -1.0f)
	{
		TLastUpdate = ofGetElapsedTimef();
		return;
	}

	if(bPressed)
	{
		Trace.addPoint(UserPos);
		Vel = Trace.getVelocity();
	}
	
	
	float TNow = ofGetElapsedTimef();
	float dt = TNow - TLastUpdate;

	if(!bPressed)
	{
		float a = -log(ScrollVelDampPerSec);
		Vel = Vel*exp(-a*dt);
		ofVec3f T = Vel * ScrollVelCoef *dt;
		Trans += T;
	}	

	ofRectangle RT = getScrolledThumbRect();
	ofPoint PDC = DispRect.getCenter();
	if(!RT.inside(PDC))
	{
		ofVec3f VReturn;

		ofPoint PTC = RT.getCenter();
		VReturn = PDC-PTC;
		//ofVec3f VReturnN = VReturn.normalize();
		
		float P = sqrt(DispRect.width*DispRect.height);
		VReturn.limit(P*0.618f);
		Vel = VReturn;
	}

	if(!bPressed)
	{
		
		notifyBrowseEvent(UserPos.x, UserPos.y, BROWSER_OVER_IMAGE);
	}
	

	debugCout("opUpdate(): ");

	TLastUpdate = ofGetElapsedTimef();
}

void ofxImageBrowser::opDraw()
{	
	if(!isOperable()) {return;};

	drawToThumbFbo();

	ThumbFbo.draw(DispRect.x,DispRect.y,DispRect.getWidth(),DispRect.getHeight());

	if(pCenterDispTgtImgKey!=NULL)
	{
		multimap<float, DispTargetImg>::iterator it;
		it = ImgDispList.find(pCenterDispTgtImgKey);
		if(it!=ImgDispList.end())
		{
			pushMSV();
			ofTranslate(DispRect.x,DispRect.y,0.0f);		
			drawDispTarget(it->second);
			popMSV();
		}		
	}	

	float fps = ofGetFrameRate();
	string FpsStr = "FPS: " + ofToString(fps);
	ofDrawBitmapString(FpsStr,5,10);
}

void ofxImageBrowser::opMove( float x, float y, float z/*=0*/, int id/*=0*/ )
{
	UserPos = ofVec3f(x,y,z);

	if(!isOperable()||!DispRect.inside(ofPoint(x,y)) ) {return;};

	notifyBrowseEvent(x, y, BROWSER_OVER_IMAGE);
}

void ofxImageBrowser::opPress( float x, float y, float z/*=0*/, int id/*=0*/ )
{
	UserPos = ofVec3f(x,y,z);

	if(!isOperable()||!DispRect.inside(ofPoint(x,y)) ) {return;};
		
	PressPos = UserPos;
	
	Trace.addPoint(UserPos);

	PointPrev = ofVec3f(x,y,z);
	bPressed = true;
	
	notifyBrowseEvent(x, y, BROWSER_CLICK_IMAGE);
}

void ofxImageBrowser::opDrag( float x,float y,float z/*=0*/, int id/*=0*/ )
{
	UserPos = ofVec3f(x,y,z);	

	if(!isOperable()||!DispRect.inside(PressPos) ) {return;};
	
	Trans += UserPos - PointPrev;
	
	PointPrev = UserPos;


	notifyBrowseEvent(x, y, BROWSER_DRAGGING_IMAGE);

	// debug 
	debugCout("opDrag(): ");
}

void ofxImageBrowser::opRelease( float x, float y, float z/*=0*/, int id/*=0*/ )
{
	UserPos = ofVec3f(x,y,z);

	if(!isOperable()||!DispRect.inside(PressPos)) {return;};
		
	PressPos = ofVec3f(0,0,-100);

	Trace.clear();

	bPressed = false;

	notifyBrowseEvent(x, y, BROWSER_RELEASE_IMAGE);
	
}

void ofxImageBrowser::resetThumbRect()
{
	float wd = DispRect.getWidth();

	thumbCols = floor(wd/ThumbSize);
	if(thumbCols<1 || ImgList.size()==0)
	{
		ThumbRect = DispRect;
		thumbRows = 0;
		return;
	}
	thumbRows = ceil(float(ImgList.size())/float(thumbCols));

	tThumbSize = wd/thumbCols;

	ThumbRect.x = DispRect.x;
	ThumbRect.y = DispRect.y;
	ThumbRect.width = DispRect.width;
	ThumbRect.height = tThumbSize * thumbRows;

	
}

void ofxImageBrowser::drawToThumbFbo()
{	
	if(thumbRows<1||thumbCols<1)
	{
		return;
	}
	
	generateDisplayList();	
	drawDisplayListOnThumbFbo();
}

void ofxImageBrowser::resetThumbFbo()
{
	ThumbFbo.allocate(DispRect.width,DispRect.height,GL_RGBA);
	clearThumbFbo();
}

void ofxImageBrowser::setThumbSize( float s )
{
	ThumbSize = s;
}

void ofxImageBrowser::setThumbScale( float s )
{
	ThumbScale = s;
}

float ofxImageBrowser::getThumbScale()
{
	return ThumbScale;
}

void ofxImageBrowser::setNormalizedControlRadius( float r )
{
	UserControlRadius = r;
}

float ofxImageBrowser::getNormalizedControlRadius()
{
	return UserControlRadius;
}

void ofxImageBrowser::setControlScaleAmt( float a )
{
	UserControlAmt = a;
}

float ofxImageBrowser::getControlScaleAmt()
{
	return UserControlAmt;
}

float ofxImageBrowser::getThumbSize()
{
	return ThumbSize;
}

void ofxImageBrowser::ThumbSizeChanged( float &s )
{	
	resetThumbRect();
	resetMovement();
}

void ofxImageBrowser::generateDisplayList()
{
	ImgDispList.clear();
	map<ofFile, ofImage>::iterator it;
	int id = 0;
	for(it=ImgList.begin();it!=ImgList.end();it++)
	{
		ofRectangle RT = ThumbRect;
		RT.x = 0;
		RT.y = 0;
		if(ScrollableHorizontal)
		{
			RT.translateX(Trans.x);
		}
		if(ScrollableVertical)
		{
			RT.translateY(Trans.y);
		}
		ofRectangle R = fp::getGridSubRect(RT,thumbCols,thumbRows,id);
		float rw,rh;
		rw = R.getWidth();
		rh = R.getHeight();

		float ScaleFitRect = 1.0f;
		float iw,ih;
		iw = it->second.getWidth();
		ih = it->second.getHeight();
		ScaleFitRect = min(rw/iw, rh/ih);

		
		ofRectangle RT2 = getScrolledThumbRect();
		ofRectangle R_ = fp::getGridSubRect(RT2,thumbCols,thumbRows,id);
		if(!R_.intersects(DispRect))
		{
			id ++;
			continue;
		}
		float Perimeter = sqrt(RT.width*RT.height);
		float HalfPm = Perimeter/2.0f;
		ofPoint R_Center = R_.getCenter();
		float Dist = UserPos.distance(R_Center);
		float NDist = Dist/HalfPm;		
		NDist = ofClamp(NDist,0.0f,UserControlRadius);
		float NControlAmt = (UserControlRadius-NDist)/UserControlRadius;
		float ScaleCtrl = 1.0f + (UserControlAmt-1.0f)*NControlAmt;

		float z = NControlAmt;
		ofVec3f RCenter(R.x+R.getWidth()/2, R.y+R.getHeight()/2,0.0f);
		float MScale = ScaleFitRect*ThumbScale*ScaleCtrl;
		ofVec3f DScale = ofVec3f(MScale,MScale,1.0f);

		ofColor C = ofColor::white;
		if(R_.inside(UserPos)&&bPressed)
		{
			C = ofColor::green;
		}

		DispTargetImg DImg(&(it->second),DScale,
			RCenter,NControlAmt,C);
		pair<float,DispTargetImg> DI(z,DImg);
		ImgDispList.insert(DI);

		if(R_.inside(UserPos))
		{
			int Num = ImgDispList.count(z);
			multimap<float, DispTargetImg>::iterator it 
				= ImgDispList.find(z);
			if(it!=ImgDispList.end())
			{
				pCenterDispTgtImgKey = z;
			}			
		}

		id++;
	}
}

void ofxImageBrowser::drawDisplayListOnThumbFbo()
{
	
	clearThumbFbo();

	map<float, DispTargetImg>::iterator itd;
	for(itd = ImgDispList.begin();itd!=ImgDispList.end();itd++)
	{
		DispTargetImg DT = itd->second;
		ThumbFbo.begin();
		drawDispTarget(DT);

		ThumbFbo.end();
	}
}

ofRectangle ofxImageBrowser::getScrolledThumbRect()
{
	ofRectangle RT = ThumbRect;
	if(ScrollableHorizontal)
	{
		RT.translateX(Trans.x);
	}
	else if(ScrollableVertical)
	{
		RT.translateY(Trans.y);
	}

	return RT;
}

void ofxImageBrowser::debugCout( string s )
{
	//cout << s << "Vel:"<< Vel << " Trans:"<< Trans << endl;
}

void ofxImageBrowser::resetMovement()
{
	Trans = ofVec3f(0,0,0);
	Vel = ofVec3f(0,0,0);
}

void ofxImageBrowser::notifyBrowseEvent( float x, float y, 
										ofxImageBrowserOperationType ET)
{
	map<ofFile, ofImage>::iterator it;
	int idImg = 0;
	for(it=ImgList.begin();it!=ImgList.end();it++)
	{	
		ofRectangle RT;
		RT = getScrolledThumbRect();		

		ofRectangle R_ = fp::getGridSubRect(RT,thumbCols,thumbRows,idImg);
		//R_.translate(DispRect.x,DispRect.y);
		if(R_.intersects(DispRect)&&R_.inside(ofPoint(x,y)))
		{
			ofxImageBrowserEventArgs E(
				ET,it->first,R_,
				&it->second,
				ofVec2f(UserPos.x,UserPos.y));		
			ofNotifyEvent(this->ImgBrowseEvent,E);
		}
		idImg++;
	}
}

void ofxImageBrowser::setOperable( bool bOperable )
{
	if(!bRunning&&bOperable)
	{
		bRunning = true;
		resetMovement();
		resetThumbFbo();
		resetThumbRect();
		return;
	}	

	bRunning = bOperable;
}

bool ofxImageBrowser::isOperable()
{
	return bRunning;
}

void ofxImageBrowser::setBackgroundColor( ofColor C )
{
	BgColor = C;
}

ofColor ofxImageBrowser::getBackgroundColor()
{
	return BgColor;
}

void ofxImageBrowser::clearThumbFbo()
{
	ThumbFbo.begin();
	ofPushStyle();
	ofDisableAlphaBlending();
	ofClear(BgColor);
	ofPopStyle();
	ThumbFbo.end();
}

void ofxImageBrowser::drawDispTarget( DispTargetImg &DT )
{
	ofPushMatrix();
	ofPushStyle();
	ofPushView();

	ofSetRectMode(OF_RECTMODE_CENTER);
	ofTranslate(DT.Pos);
	ofScale(DT.Scale.x, DT.Scale.y, DT.Scale.z);		

	float iw,ih;
	iw = DT.pImg->getWidth();
	ih = DT.pImg->getHeight();

	ofNoFill();
	ofSetLineWidth(DT.NDistToUserPos*3.5f);
	float A(DT.NDistToUserPos);
	A = 255.0f*A;		
	ofColor C = DT.BoundColor;
	C.a = A;
	ofSetColor(C);		
	ofRectangle RBound(0,0,iw,ih);
	RBound.scaleFromCenter(1.005f);
	ofRect(RBound);

	ofFill();
	ofSetColor(ofColor::white);
	DT.pImg->draw(0,0,iw,ih);

	ofPopMatrix();
	ofPopStyle();
	ofPopView();
}

void ofxImageBrowser::pushMSV()
{
	ofPushMatrix();
	ofPushStyle();
	ofPushView();
}

void ofxImageBrowser::popMSV()
{
	ofPopMatrix();
	ofPopStyle();	
	ofPopView();	
}

bool ofxImageBrowser::inside( int x, int y ) const
{
	bool bInside = DispRect.inside(ofPoint(x,y));

	return bInside;
}

//ofEvent <ofxImageBrowserEventArgs> ofxImageBrowser::ImgBrowseEvent;

ofxImageBrowserEventArgs::ofxImageBrowserEventArgs( 
	ofxImageBrowserOperationType Op, 
	ofFile ImgF, 
	ofRectangle thumbRect, 
	ofImage* ptrThumb,
	ofVec2f pos)
{
	Type = Op;
	ImgFile = ImgF;
	ThumbRect = thumbRect;
	pThumb = ptrThumb;
	Pos = pos;
}

ofxImageBrowserEventArgs::ofxImageBrowserEventArgs()
{
	Type = BROWSER_NONE;
	ThumbRect = ofRectangle(0,0,0,0);
	pThumb = NULL;
}

ofxImageBrowser::DispTargetImg::DispTargetImg( 
	ofImage* ppImg, ofVec3f scl, ofVec3f pos, float ndist,
	ofColor C /*= ofColor::white*/ ):
		pImg(ppImg),Scale(scl),Pos(pos),
		NDistToUserPos(ndist),BoundColor(C)
{}
