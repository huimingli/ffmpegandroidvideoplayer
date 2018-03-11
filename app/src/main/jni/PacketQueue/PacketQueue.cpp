#include "PacketQueue.h"


static bool isExit = false;
PacketQueue::PacketQueue()
{
	size = 0;
}

PacketQueue::~PacketQueue() {
	std::lock_guard<std::mutex> locker(mutex);
	isExit = true;	 
}

//************************************
// Method:    enQueue
// FullName:  PacketQueue::enQueue
// Access:    public 
// Returns:   bool
// Qualifier:���������
// Parameter: const AVPacket packet
//************************************
bool PacketQueue::enQueue(const AVPacket packet)
{
	std::unique_lock<std::mutex> locker(mutex);
	queue.push(packet);
	size += packet.size;
	cond.notify_one();
	return true;
}

//************************************
// Method:    deQueue
// FullName:  PacketQueue::deQueue
// Access:    public 
// Returns:   AVPacket
// Qualifier:�����г���
//************************************
AVPacket PacketQueue::deQueue()
{
	bool ret = false;
	AVPacket pkt;
	std::unique_lock<std::mutex> locker(mutex);

		if (!queue.empty())
		{
			pkt = queue.front();
			queue.pop();
			size -= pkt.size;		 			
			ret = true;
		 
		}	 
		else
		{			
			cond.wait(locker);
		} 
	
	return pkt;
}

//************************************
// Method:    getPacketSize
// FullName:  PacketQueue::getPacketSize
// Access:    public 
// Returns:   Uint32
// Qualifier:��ȡ����С
//************************************
Uint32 PacketQueue::getPacketSize()
{
	std::lock_guard<std::mutex> locker(mutex);
  	return size;
}

//************************************
// Method:    queueFlush
// FullName:  PacketQueue::queueFlush
// Access:    public 
// Returns:   void
// Qualifier:��ն���
//************************************
void PacketQueue::queueFlush() {
	 while (!queue.empty())
	 {	
		 AVPacket pkt = deQueue();
		 av_packet_unref(&pkt);
	 }
}