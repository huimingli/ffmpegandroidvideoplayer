#include "Audio.h"

#include <fstream>
#include <jni.h>
#include <string>
#include <android/log.h>
extern "C" {
#include <libswresample/swresample.h>
}

#include "SDL.h"
#include "SDL_main.h"
static int audioVolume = 64;
Audio::Audio()
{
	SDL_Init(SDL_INIT_AUDIO);
	audioContext = nullptr;
	streamIndex = -1;
	stream = nullptr;
	audioClock = 0;
	audioBuff = new uint8_t[BUFFERSIZE];
	audioBuffSize = 0;
	audioBuffIndex = 0;
}

//************************************
// Method:    r2d
// FullName:  r2d
// Access:    public static
// Returns:   double
// Qualifier:����ʱ���׼
// Parameter: AVRational r
//************************************
static double r2d(AVRational r)
{
	return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
}

Audio::~Audio()
{
	audioClose();
	SDL_Quit();
	if (audioBuff)
		delete[] audioBuff;
}

//************************************
// Method:    audioClose
// FullName:  Audio::audioClose
// Access:    public
// Returns:   bool
// Qualifier:�ر���Ƶ
//************************************
bool Audio::audioClose() {
	SDL_CloseAudio();
	return true;
}

//************************************
// Method:    audioPlay
// FullName:  Audio::audioPlay
// Access:    public
// Returns:   bool
// Qualifier:������Ƶ
//************************************
bool Audio::audioPlay()
{

	SDL_AudioSpec desired;
	desired.freq = audioContext->sample_rate;
	desired.channels = audioContext->channels;
	desired.format = AUDIO_S16SYS;
	desired.samples = 1024;
	desired.silence = 0;
	desired.userdata = this;
	desired.callback = audioCallback;
	if (SDL_OpenAudio(&desired, nullptr) < 0)
	{
		__android_log_print(ANDROID_LOG_DEBUG, "DEBUG", "audio error:%s",SDL_GetError());
		return false;
	}
	SDL_PauseAudio(0); // playing

	return true;
}

//************************************
// Method:    getCurrentAudioClock
// FullName:  Audio::getCurrentAudioClock
// Access:    public
// Returns:   double
// Qualifier:��ȡ��ǰ��Ƶʱ��
//************************************
double Audio::getCurrentAudioClock()
{
	int hw_buf_size = audioBuffSize - audioBuffIndex;
	int bytes_per_sec = stream->codecpar->sample_rate * audioContext->channels * 2;
	double pts = audioClock - static_cast<double>(hw_buf_size) / bytes_per_sec;
	return pts;
}

//************************************
// Method:    getStreamIndex
// FullName:  Audio::getStreamIndex
// Access:    public
// Returns:   int
// Qualifier:��ȡ���±�
//************************************
int Audio::getStreamIndex()
{
	return streamIndex;
}

//************************************
// Method:    setStreamIndex
// FullName:  Audio::setStreamIndex
// Access:    public
// Returns:   void
// Qualifier:�������±�
// Parameter: const int streamIndex
//************************************
void Audio::setStreamIndex(const int streamIndex)
{
	this->streamIndex = streamIndex;
}

//************************************
// Method:    getAudioQueueSize
// FullName:  Audio::getAudioQueueSize
// Access:    public
// Returns:   int
// Qualifier:��ȡ��Ƶ���д�С
//************************************
int Audio::getAudioQueueSize()
{
	return audiaPackets.getPacketSize();
}

//************************************
// Method:    enqueuePacket
// FullName:  Audio::enqueuePacket
// Access:    public
// Returns:   void
// Qualifier:��Ƶ�����
// Parameter: const AVPacket pkt
//************************************
void Audio::enqueuePacket(const AVPacket pkt)
{
	audiaPackets.enQueue(pkt);
}

//************************************
// Method:    dequeuePacket
// FullName:  Audio::dequeuePacket
// Access:    public
// Returns:   AVPacket
// Qualifier:��Ƶ����
//************************************
AVPacket Audio::dequeuePacket()
{
	return audiaPackets.deQueue();
}

//************************************
// Method:    getAudioBuff
// FullName:  Audio::getAudioBuff
// Access:    public
// Returns:   QT_NAMESPACE::uint8_t*
// Qualifier:��ȡ��Ƶ����
//************************************
uint8_t* Audio::getAudioBuff()
{
	return audioBuff;
}

//************************************
// Method:    setAudioBuff
// FullName:  Audio::setAudioBuff
// Access:    public
// Returns:   void
// Qualifier:������Ƶ����
// Parameter: uint8_t * & audioBuff
//************************************
void Audio::setAudioBuff(uint8_t*&  audioBuff)
{
	this->audioBuff = audioBuff;
}

//************************************
// Method:    getAudioBuffSize
// FullName:  Audio::getAudioBuffSize
// Access:    public
// Returns:   uint32_t
// Qualifier:��ȡ��Ƶ�����С
//************************************
uint32_t Audio::getAudioBuffSize()
{
	return audioBuffSize;
}

//************************************
// Method:    setAudioBuffSize
// FullName:  Audio::setAudioBuffSize
// Access:    public
// Returns:   void
// Qualifier: ���û����С
// Parameter: uint32_t audioBuffSize
//************************************
void Audio::setAudioBuffSize(uint32_t audioBuffSize)
{
	this->audioBuffSize = audioBuffSize;
}

//************************************
// Method:    getAudioBuffIndex
// FullName:  Audio::getAudioBuffIndex
// Access:    public
// Returns:   uint32_t
// Qualifier:��ȡ��Ƶ�����±�
//************************************
uint32_t Audio::getAudioBuffIndex()
{
	return audioBuffIndex;
}

//************************************
// Method:    setAudioBuffIndex
// FullName:  Audio::setAudioBuffIndex
// Access:    public
// Returns:   void
// Qualifier:������Ƶ������±�
// Parameter: uint32_t audioBuffIndex
//************************************
void Audio::setAudioBuffIndex(uint32_t audioBuffIndex)
{
	this->audioBuffIndex = audioBuffIndex;
}

//************************************
// Method:    getAudioClock
// FullName:  Audio::getAudioClock
// Access:    public
// Returns:   double
// Qualifier:��ȡ��Ƶʱ��
//************************************
double Audio::getAudioClock()
{
	return audioClock;
}

//************************************
// Method:    setAudioClock
// FullName:  Audio::setAudioClock
// Access:    public
// Returns:   void
// Qualifier:������Ƶʱ��
// Parameter: const double & audioClock
//************************************
void Audio::setAudioClock(const double & audioClock)
{
	this->audioClock = audioClock;
}

//************************************
// Method:    getStream
// FullName:  Audio::getStream
// Access:    public
// Returns:   AVStream *
// Qualifier:��ȡ��Ƶ��
//************************************
AVStream * Audio::getStream()
{
	return this->stream;
}

//************************************
// Method:    setStream
// FullName:  Audio::setStream
// Access:    public
// Returns:   void
// Qualifier:������Ƶ��
// Parameter: AVStream * & stream
//************************************
void Audio::setStream(AVStream *& stream)
{
	this->stream = stream;
}

//************************************
// Method:    getAVCodecContext
// FullName:  Audio::getAVCodecContext
// Access:    public
// Returns:   AVCodecContext *
// Qualifier:��ȡ������������
//************************************
AVCodecContext * Audio::getAVCodecContext()
{
	return this->audioContext;
}

//************************************
// Method:    setAVCodecContext
// FullName:  Audio::setAVCodecContext
// Access:    public
// Returns:   void
// Qualifier:������Ƶ������������
// Parameter: AVCodecContext * audioContext
//************************************
void Audio::setAVCodecContext(AVCodecContext * audioContext)
{
	this->audioContext = audioContext;
}

//************************************
// Method:    getIsPlaying
// FullName:  Audio::getIsPlaying
// Access:    public
// Returns:   bool
// Qualifier:��ȡ����״̬
//************************************
bool Audio::getIsPlaying()
{
	return isPlay;
}

//************************************
// Method:    setPlaying
// FullName:  Audio::setPlaying
// Access:    public
// Returns:   void
// Qualifier:���ò���״̬
// Parameter: bool isPlaying
//************************************
void Audio::setPlaying(bool isPlaying)
{
	this->isPlay = isPlaying;
}

//************************************
// Method:    clearPacket
// FullName:  Audio::clearPacket
// Access:    public
// Returns:   void
// Qualifier:������Ƶ������
//************************************
void Audio::clearPacket()
{
	audiaPackets.queueFlush();
}

//************************************
// Method:    setVolume
// FullName:  Audio::setVolume
// Access:    public
// Returns:   void
// Qualifier: ��������
// Parameter: int volume
//************************************
void Audio::setVolume(int volume) {
	audioVolume = volume;
}
static std::mutex mutex;
/**
* ���豸����audio���ݵĻص�����
*/
void audioCallback(void* userdata, Uint8 *stream, int len) {

	Audio  *audio = Audio::getInstance();

	SDL_memset(stream, 0, len);

	int audio_size = 0;
	int len1 = 0;
	while (len > 0)// ���豸���ͳ���Ϊlen������
	{
		if (audio->getAudioBuffIndex() >= audio->getAudioBuffSize()) // ��������������
		{
			// ��packet�н�������
			audio_size = audioDecodeFrame(audio, audio->getAudioBuff(), sizeof(audio->getAudioBuff()));
			if (audio_size < 0) // û�н��뵽���ݻ�������0
			{
				audio->setAudioBuffSize(0);
				memset(audio->getAudioBuff(), 0, audio->getAudioBuffSize());
			}
			else
				audio->setAudioBuffSize(audio_size);

			audio->setAudioBuffIndex(0);
		}
		len1 = audio->getAudioBuffSize() - audio->getAudioBuffIndex(); // ��������ʣ�µ����ݳ���
		if (len1 > len) // ���豸���͵����ݳ���Ϊlen
			len1 = len;

		SDL_MixAudio(stream, audio->getAudioBuff() + audio->getAudioBuffIndex(), len, audioVolume);

		len -= len1;
		stream += len1;
		audio->setAudioBuffIndex(audio->getAudioBuffIndex()+len1);
	}
}

/**
* ����Avpacket�е�������䵽����ռ�
*/
int audioDecodeFrame(Audio*audio, uint8_t *audioBuffer, int bufferSize) {
	AVFrame *frame = av_frame_alloc();
	int data_size = 0;
	std::lock_guard<std::mutex> locker(mutex);
	AVPacket pkt = audio->dequeuePacket();
	SwrContext *swr_ctx = nullptr;
	static double clock = 0;

	/*if (quit)
		return -1;*/
	if (pkt.size <= 0||pkt.buf->size<=0)
	{
		return -1;
	}

	if (pkt.pts != AV_NOPTS_VALUE)
	{
		if (audio->getStream() == nullptr)
			return -1;
		audio->setAudioClock(av_q2d(audio->getStream()->time_base) * pkt.pts);
	}
	if (audio->getAVCodecContext() == nullptr)
		return -1;
	int ret = avcodec_send_packet(audio->getAVCodecContext(), &pkt);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
		return -1;

	ret = avcodec_receive_frame(audio->getAVCodecContext(), frame);
	if (ret < 0 && ret != AVERROR_EOF)
		return -1;
	int p = (frame->pts *r2d(audio->getStream()->time_base)) * 1000;
	Audio::getInstance()->pts = p;
	// ����ͨ������channel_layout
	if (frame->channels > 0 && frame->channel_layout == 0)
		frame->channel_layout = av_get_default_channel_layout(frame->channels);
	else if (frame->channels == 0 && frame->channel_layout > 0)
		frame->channels = av_get_channel_layout_nb_channels(frame->channel_layout);

	AVSampleFormat dst_format = AV_SAMPLE_FMT_S16;//av_get_packed_sample_fmt((AVSampleFormat)frame->format);
	Uint64 dst_layout = av_get_default_channel_layout(frame->channels);
	// ����ת������
	swr_ctx = swr_alloc_set_opts(nullptr, dst_layout, dst_format, frame->sample_rate,
		frame->channel_layout, (AVSampleFormat)frame->format, frame->sample_rate, 0, nullptr);
	if (!swr_ctx || swr_init(swr_ctx) < 0)
		return -1;

	// ����ת�����sample���� a * b / c
	uint64_t dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples, frame->sample_rate, frame->sample_rate, AVRounding(1));
	// ת��������ֵΪת�����sample����
	int nb = swr_convert(swr_ctx, &audioBuffer, static_cast<int>(dst_nb_samples), (const uint8_t**)frame->data, frame->nb_samples);
	data_size = frame->channels * nb * av_get_bytes_per_sample(dst_format);

	// ÿ������Ƶ���ŵ��ֽ��� sample_rate * channels * sample_format(һ��sampleռ�õ��ֽ���)
	if (audio->getStream()->codec == nullptr)
		return -1;
	__android_log_print(ANDROID_LOG_DEBUG, "DEBUG", "audio read");
	audio->setAudioClock(audio->getAudioClock()+static_cast<double>(data_size) / (2 * audio->getStream()->codec->channels * audio->getStream()->codec->sample_rate));
	av_frame_unref(frame);
	av_frame_free(&frame);
	swr_free(&swr_ctx);
	return data_size;
}