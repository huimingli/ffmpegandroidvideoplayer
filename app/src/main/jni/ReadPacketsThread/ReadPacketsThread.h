#pragma once
#include <mutex>
#include <thread>
class ReadPacketsThread 
{
	 
public:
	static ReadPacketsThread * getInstance() {
		static ReadPacketsThread rpt;
		return &rpt;
	}
	void run();
	~ReadPacketsThread();
	bool getIsPlaying();
	float currentPos = 0;
	bool isSeek = false;
	void setPlaying(bool isPlaying);
	void receivePos(float pos);
private:
	std::mutex mutex;
	ReadPacketsThread();
	bool isPlay = false;
};

