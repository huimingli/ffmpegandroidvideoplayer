
#pragma once

#include <queue>

#include <SDL.h>
#include <condition_variable>
#include <mutex>
extern "C"{

#include <libavcodec/avcodec.h>

}

class PacketQueue
{
public:
	PacketQueue();
	bool enQueue(const AVPacket packet);
	AVPacket deQueue();
	Uint32 getPacketSize();
	void queueFlush();
	~PacketQueue();
private:
	std::queue<AVPacket> queue;
	Uint32    size;
	std::mutex mutex;
	std::condition_variable cond;

	
};

 