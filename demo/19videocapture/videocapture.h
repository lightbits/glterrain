#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H

class VideoCapture
{
public:
	VideoCapture() : pixels(nullptr) { }
	~VideoCapture() { if(pixels != nullptr) delete[] pixels; }
	void begin();
	void captureFrame();
	void dumpFrames();
private:
	int frame;
	int maxFrames;
	double frameTime;
	unsigned char *pixels;

};

//int videoFPS = 2;
//int videoDuration = 5;
//double videoTimer = 0.0;
//double frameTime = 1.0 / double(videoFPS);
//int videoFrameCount = videoFPS * videoDuration;
//int currentVideoFrame = 0;
//unsigned int videoFrameSize = 640 * 480 * 3;
//unsigned char *videoMemoryPtr = new unsigned char[videoFrameSize * videoFrameCount];

//videoTimer += timer.getDelta();
//if(videoTimer > frameTime && currentVideoFrame < videoFrameCount)
//{
//	videoTimer -= frameTime;
//	glReadPixels(0, 0, 640, 480, GL_RGB, GL_UNSIGNED_BYTE, videoMemoryPtr + currentVideoFrame * videoFrameSize);
//	currentVideoFrame++;
//}

// save frames
//for(int i = 0; i < videoFrameCount; ++i)
//{
//	std::stringstream name;
//	name<<"capture"<<i<<".png";
//	if(!savePNG(name.str().c_str(), videoMemoryPtr + i * videoFrameSize, 640, 480))
//		break;
//}
//
//delete[] videoMemoryPtr;

bool savePNG(const char *filename, 
			 const unsigned char *pixels,
			 unsigned int width, 
			 unsigned int height);

#endif