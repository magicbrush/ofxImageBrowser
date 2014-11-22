#include "ofxThreadedImageLoader_.h"
#include <sstream>
ofxThreadedImageLoader_::ofxThreadedImageLoader_() 
:ofThread()
{
	nextID = 0;
    ofAddListener(ofEvents().update, this, &ofxThreadedImageLoader_::update);
	ofAddListener(ofURLResponseEvent(),this,&ofxThreadedImageLoader_::urlResponse);
    
    startThread();
    lastUpdate = 0;

	RLimits = ofVec2f(4096,4096);
}

ofxThreadedImageLoader_::~ofxThreadedImageLoader_(){
	condition.signal();
    ofRemoveListener(ofEvents().update, this, &ofxThreadedImageLoader_::update);
	ofRemoveListener(ofURLResponseEvent(),this,&ofxThreadedImageLoader_::urlResponse);
}

// Load an image from disk.
//--------------------------------------------------------------
void ofxThreadedImageLoader_::loadFromDisk(ofImage& image, string filename) {
	nextID++;
	ofImageLoaderEntry entry(image, OF_LOAD_FROM_DISK);
	entry.filename = filename;
	entry.id = nextID;
	entry.image->setUseTexture(false);
	entry.name = filename;
    
    lock();
    images_to_load_buffer.push_back(entry);
    condition.signal();
    unlock();
}


// Load an url asynchronously from an url.
//--------------------------------------------------------------
void ofxThreadedImageLoader_::loadFromURL(ofImage& image, string url) {
	nextID++;
	ofImageLoaderEntry entry(image, OF_LOAD_FROM_URL);
	entry.url = url;
	entry.id = nextID;
	entry.image->setUseTexture(false);	
	entry.name = "image" + ofToString(entry.id);

    lock();
	images_to_load_buffer.push_back(entry);
    condition.signal();
    unlock();
}


// Reads from the queue and loads new images.
//--------------------------------------------------------------
void ofxThreadedImageLoader_::threadedFunction() {
    deque<ofImageLoaderEntry> images_to_load;

	while( isThreadRunning() ) {
		lock();
		if(images_to_load_buffer.empty()) condition.wait(mutex);
		images_to_load.insert( images_to_load.end(),
							images_to_load_buffer.begin(),
							images_to_load_buffer.end() );

		images_to_load_buffer.clear();
		unlock();
        
        
        while( !images_to_load.empty() ) {
            ofImageLoaderEntry  & entry = images_to_load.front();
            
            if(entry.type == OF_LOAD_FROM_DISK) {
                if(! entry.image->loadImage(entry.filename) )  { 
                    ofLogError("ofxThreadedImageLoader_") << "couldn't load file: \"" << entry.filename << "\"";
                }
                
                lock();
                images_to_update.push_back(entry);
                unlock();
            }else if(entry.type == OF_LOAD_FROM_URL) {
                lock();
                images_async_loading.push_back(entry);
                unlock();
                
                ofLoadURLAsync(entry.url, entry.name);
            }

    		images_to_load.pop_front();
        }
	}
}


// When we receive an url response this method is called; 
// The loaded image is removed from the async_queue and added to the
// update queue. The update queue is used to update the texture.
//--------------------------------------------------------------
void ofxThreadedImageLoader_::urlResponse(ofHttpResponse & response) {
	if(response.status == 200) {
		lock();
		
		// Get the loaded url from the async queue and move it into the update queue.
		entry_iterator it = getEntryFromAsyncQueue(response.request.name);
		if(it != images_async_loading.end()) {
			(*it).image->loadImage(response.data);
			constrainResolution((*it).image);
			images_to_update.push_back(*it);
			images_async_loading.erase(it);
		}
		
		unlock();
	}else{
		// log error.
		ofLogError("ofxThreadedImageLoader_") << "couldn't load url, response status: " << response.status;
		ofRemoveURLRequest(response.request.getID());
		// remove the entry from the queue
		lock();
		entry_iterator it = getEntryFromAsyncQueue(response.request.name);
		if(it != images_async_loading.end()) {
			images_async_loading.erase(it);
		}
		unlock();
	}
}


// Check the update queue and update the texture
//--------------------------------------------------------------
void ofxThreadedImageLoader_::update(ofEventArgs & a){
    
    // Load 1 image per update so we don't block the gl thread for too long
    
    lock();
	if (!images_to_update.empty()) {

		ofImageLoaderEntry entry = images_to_update.front();

		const ofPixels& pix = entry.image->getPixelsRef();
		entry.image->getTextureReference().allocate(
				 pix.getWidth()
				,pix.getHeight()
				,ofGetGlInternalFormat(pix)
		);
		
		entry.image->setUseTexture(true);
		entry.image->update();

		constrainResolution(entry.image);

		images_to_update.pop_front();
	}
    unlock();
}


// Find an entry in the aysnc queue.
//   * private, no lock protection, is private function
//--------------------------------------------------------------
ofxThreadedImageLoader_::entry_iterator ofxThreadedImageLoader_::getEntryFromAsyncQueue(string name) {
	entry_iterator it = images_async_loading.begin();
	for(;it != images_async_loading.end();it++) {
		if((*it).name == name) {
			return it;
		}
	}
	return images_async_loading.end();
}

void ofxThreadedImageLoader_::constrainResolution( ofImage* image )
{
	float w,h;
	w = image->getWidth();
	h = image->getHeight();

	bool bResize = false;
	if(w>RLimits.x)
	{
		h = h*RLimits.x/w;
		w = RLimits.x;		
		bResize = true;
	}

	if(h>RLimits.y)
	{
		w = w*RLimits.y/h;
		h = RLimits.y;
	}

	image->resize(w,h);
}

void ofxThreadedImageLoader_::setResolutionLimits( ofVec2f rLimits )
{
	if(rLimits.x>1&&rLimits.y>1)
	{
		RLimits = rLimits;
	}
}
