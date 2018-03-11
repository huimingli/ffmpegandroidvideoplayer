#include <thread>
#include "Video.h"
static bool isExit = false;
Video::Video()
{
	frameTimer = 0.0;
	frameLastDelay = 0.0;
	frameLastPts = 0.0;
	videoClock = 0.0;
	videoPackets = new PacketQueue;
}


Video::~Video()
{
	std::lock_guard<std::mutex> locker(mutex);
	delete videoPackets;
	isExit = true;
}

//************************************
// Method:    getStreamIndex
// FullName:  Video::getStreamIndex
// Access:    public 
// Returns:   int
// Qualifier: ��ȡ���±�
//************************************
int Video::getStreamIndex()
{
	return streamIndex;
}

//************************************
// Method:    setStreamIndex
// FullName:  Video::setStreamIndex
// Access:    public 
// Returns:   void
// Qualifier:�������±�
// Parameter: const int & streamIndex
//************************************
void Video::setStreamIndex(const int &streamIndex)
{
	this->streamIndex = streamIndex;
}

//************************************
// Method:    getVideoQueueSize
// FullName:  Video::getVideoQueueSize
// Access:    public 
// Returns:   int
// Qualifier:��ȡ��Ƶ���д�С
//************************************
int Video::getVideoQueueSize()
{
	return videoPackets->getPacketSize();
}

//************************************
// Method:    enqueuePacket
// FullName:  Video::enqueuePacket
// Access:    public 
// Returns:   void
// Qualifier: �����
// Parameter: const AVPacket & pkt
//************************************
void Video::enqueuePacket(const AVPacket &pkt)
{
	videoPackets->enQueue(pkt);
}

//************************************
// Method:    dequeueFrame
// FullName:  Video::dequeueFrame
// Access:    public 
// Returns:   AVFrame *
// Qualifier: ֡���г���
//************************************
AVFrame * Video::dequeueFrame()
{
	return frameQueue.deQueue();
}

//************************************
// Method:    synchronizeVideo
// FullName:  Video::synchronizeVideo
// Access:    public 
// Returns:   double
// Qualifier: ����ͬ����Ƶ�Ĳ���ʱ��
// Parameter: AVFrame * & srcFrame
// Parameter: double & pts
//************************************
double Video::synchronizeVideo(AVFrame *&srcFrame, double &pts)
{
	double frameDelay;
	if (pts != 0)
		videoClock = pts; // Get pts,then set video clock to it
	else
		pts = videoClock; // Don't get pts,set it to video clock
	frameDelay = av_q2d(stream->codec->time_base);
	frameDelay += srcFrame->repeat_pict * (frameDelay * 0.5);
	videoClock += frameDelay;
	return pts;
}

//************************************
// Method:    getVideoStream
// FullName:  Video::getVideoStream
// Access:    public 
// Returns:   AVStream *
// Qualifier: ��ȡ��Ƶ��
//************************************
AVStream * Video::getVideoStream()
{
	return stream;
}

//************************************
// Method:    setVideoStream
// FullName:  Video::setVideoStream
// Access:    public 
// Returns:   void
// Qualifier:������Ƶ��
// Parameter: AVStream * & videoStream
//************************************
void Video::setVideoStream(AVStream *& videoStream)
{
	this->stream = videoStream;
}

//************************************
// Method:    getAVCodecCotext
// FullName:  Video::getAVCodecCotext
// Access:    public 
// Returns:   AVCodecContext *
// Qualifier:��ȡ��Ƶ������������
//************************************
AVCodecContext * Video::getAVCodecCotext()
{
	return this->videoContext;
}

//************************************
// Method:    setAVCodecCotext
// FullName:  Video::setAVCodecCotext
// Access:    public 
// Returns:   void
// Qualifier:������Ƶ������������
// Parameter: AVCodecContext * avCodecContext
//************************************
void Video::setAVCodecCotext(AVCodecContext * avCodecContext)
{
	this->videoContext = avCodecContext;
}

//************************************
// Method:    setFrameTimer
// FullName:  Video::setFrameTimer
// Access:    public 
// Returns:   void
// Qualifier:����֡ʱ��
// Parameter: const double & frameTimer
//************************************
void Video::setFrameTimer(const double & frameTimer)
{
	this->frameTimer = frameTimer;
}
 

//************************************
// Method:    run
// FullName:  Video::run
// Access:    public 
// Returns:   void
// Qualifier:��Ƶ��֡�̴߳�����
//************************************
void Video::run()
{
	 AVFrame * frame = av_frame_alloc();
	double pts;

	while (!isExit)
	{
		std::unique_lock<std::mutex> locker(mutex);
		if (frameQueue.getQueueSize() >= FrameQueue::capacity) {//��Ƶ֡����30֡�͵ȴ�����
			locker.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}			
		if (videoPackets->getPacketSize() == 0) {//û֡�ȴ�֡���
			locker.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
        AVPacket pkt = videoPackets->deQueue();//����
		int ret = avcodec_send_packet(videoContext, &pkt);
		if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
			continue;
		}

		av_packet_unref(&pkt);
        av_free_packet(&pkt);
		ret = avcodec_receive_frame(videoContext, frame);
		if (ret < 0 && ret != AVERROR_EOF) {
			continue;
		}
			
		if ((pts = av_frame_get_best_effort_timestamp(frame)) == AV_NOPTS_VALUE)
			pts = 0;
		pts *= av_q2d(stream->time_base);
		pts = synchronizeVideo(frame, pts);//ͬ����Ƶ����ʱ��
		frame->opaque = &pts;	
		frameQueue.enQueue(frame);//֡���
		av_frame_unref(frame);
	}
	av_frame_free(&frame);
}

//************************************
// Method:    getFrameTimer
// FullName:  Video::getFrameTimer
// Access:    public 
// Returns:   double
// Qualifier:��ȡ֡ʱ��
//************************************
double Video::getFrameTimer()
{
	return frameTimer;
}

//************************************
// Method:    setFrameLastPts
// FullName:  Video::setFrameLastPts
// Access:    public 
// Returns:   void
// Qualifier:��ȡ��һ֡�Ĳ���ʱ��
// Parameter: const double & frameLastPts
//************************************
void Video::setFrameLastPts(const double & frameLastPts)
{
	this->frameLastPts = frameLastPts;
}

//************************************
// Method:    getFrameLastPts
// FullName:  Video::getFrameLastPts
// Access:    public 
// Returns:   double
// Qualifier:��ȡ��һ֡�Ĳ���ʱ��
//************************************
double Video::getFrameLastPts()
{
	return frameLastPts;
}

//************************************
// Method:    setFrameLastDelay
// FullName:  Video::setFrameLastDelay
// Access:    public 
// Returns:   void
// Qualifier:������һ֡����ʱ
// Parameter: const double & frameLastDelay
//************************************
void Video::setFrameLastDelay(const double & frameLastDelay)
{
	this->frameLastDelay = frameLastDelay;
}

//************************************
// Method:    getFrameLastDelay
// FullName:  Video::getFrameLastDelay
// Access:    public 
// Returns:   double
// Qualifier:��ȡ��һ֡��ʱ
//************************************
double Video::getFrameLastDelay()
{
	return frameLastDelay;
}

//************************************
// Method:    setVideoClock
// FullName:  Video::setVideoClock
// Access:    public 
// Returns:   void
// Qualifier:������Ƶʱ��
// Parameter: const double & videoClock
//************************************
void Video::setVideoClock(const double & videoClock)
{
	this->videoClock = videoClock;
}

//************************************
// Method:    getVideoClock
// FullName:  Video::getVideoClock
// Access:    public 
// Returns:   double
// Qualifier:��ȡ��Ƶʱ��
//************************************
double Video::getVideoClock()
{
	return videoClock;
}

//************************************
// Method:    getVideoFrameSiez
// FullName:  Video::getVideoFrameSiez
// Access:    public 
// Returns:   int
// Qualifier:��ȡ֡��С
//************************************
int Video::getVideoFrameSiez()
{
	return frameQueue.getQueueSize();
}

//************************************
// Method:    clearFrames
// FullName:  Video::clearFrames
// Access:    public 
// Returns:   void
// Qualifier:���֡����
//************************************
void Video::clearFrames()
{
	frameQueue.queueFlush();
}

//************************************
// Method:    clearPackets
// FullName:  Video::clearPackets
// Access:    public 
// Returns:   void
// Qualifier:��հ�����
//************************************
void Video::clearPackets()
{
	videoPackets->queueFlush();
}
