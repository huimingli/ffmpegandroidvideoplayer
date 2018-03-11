
#include "FrameQueue.h"
FrameQueue::FrameQueue()
{
}

//************************************
// Method:    enQueue
// FullName:  FrameQueue::enQueue
// Access:    public
// Returns:   bool
// Qualifier: ֡�������
// Parameter: const AVFrame * frame
//************************************
bool FrameQueue::enQueue(const AVFrame* frame)
{
    AVFrame* p = av_frame_alloc();

    int ret = av_frame_ref(p, frame);
    if (ret < 0)
        return false;
    p->opaque = (void *)new double(*(double*)p->opaque); //��һ��ָ�����һ���ֲ��ı������������·���pts�ռ�
    std::unique_lock<std::mutex> guard(mutex);
    queue.push(p);
    cond.notify_one();
    return true;
}

//************************************
// Method:    deQueue
// FullName:  FrameQueue::deQueue
// Access:    public
// Returns:   AVFrame *
// Qualifier:֡����
//************************************
AVFrame * FrameQueue::deQueue()
{
    bool ret = true;
    AVFrame *tmp;
    std::unique_lock<std::mutex> guard(mutex);
    while (true)
    {
        if (!queue.empty())
        {
            tmp = queue.front();
            queue.pop();
            ret = true;
            break;
        }
        else
        {
            cond.wait(guard);
        }
    }

    return tmp;
}

//************************************
// Method:    getQueueSize
// FullName:  FrameQueue::getQueueSize
// Access:    public
// Returns:   int
// Qualifier:��ȡ���д�С
//************************************
int FrameQueue::getQueueSize()
{
    return queue.size();
}

//************************************
// Method:    queueFlush
// FullName:  FrameQueue::queueFlush
// Access:    public
// Returns:   void
// Qualifier:���֡����
//************************************
void FrameQueue::queueFlush() {
    while (!queue.empty())
    {
        AVFrame *frame = deQueue();
        av_frame_unref(frame);
        av_frame_free(&frame);
    }
}