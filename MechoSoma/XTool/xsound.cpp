#include "xsound.h"

#include <memory>
#include <vector>

#include <clunk/clunk.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <portaudio.h>

#include "filesystem.h"

class SoundManager final
{
 public:
  SoundManager(int maxHZ, int digMode, int channels) : _channels(channels)
  {
    if (digMode != DIG_F_STEREO_16)
    {
      throw std::runtime_error("only DIG_F_STEREO_16 supported");
    }

    const auto result = Pa_Initialize();
    if (result != paNoError)
    {
      throw std::runtime_error("Pa_Initialize");
    }

    setupMainStream(maxHZ);

    _context.init(clunk::AudioSpec{clunk::AudioSpec::Format::S16, maxHZ, 2});
    _context.set_max_sources(channels);
    _object = _context.create_object();

    startStream();
  }

  ~SoundManager()
  {
    if (_mainStream != nullptr)
    {
      Pa_StopStream(_mainStream);
      Pa_CloseStream(_mainStream);
    }

    Pa_Terminate();
  }

  clunk::Context &get_context()
  {
    return _context;
  }

  void play(clunk::Sample *sample, int channel, int priority, int cropos, int flags)
  {
    if (cropos == 255)
    {
      if (!_object->playing(sample->name) || _channels[channel].priority > priority)
      {
        stop_sound(channel);
        auto source = new clunk::Source(sample, false, clunk::v3<float>(), 1, 1, _channels[channel].pan * 0.0001f);
        _object->play(sample->name, source);
        if (flags & DS_LOOPING)
        {
          _object->set_loop(sample->name, true);
        }
      }
      else
      {
        return;
      }
    }
    else
    {
      if (_channels[channel].sound)
      {
        if (_object->playing(_channels[channel].sound->name) && _channels[channel].priority < priority)
        {
          return;
        }
        stop_sound(channel);
      }
      auto source = new clunk::Source(sample, false, clunk::v3<float>(), 1, 1, _channels[channel].pan * 0.0001f);
      _object->play(sample->name, source);
    }

    _channels[channel].priority = priority;
    _channels[channel].flags = flags;
    _channels[channel].cropos = cropos;
    _channels[channel].sound = sample;
  }

  void set_sound_volume(int channel, int volume)
  {
    float f_volume = volume + 10000;
    f_volume *= 0.0001;

    if (_channels[channel].sound)
    {
      _channels[channel].volume = volume;
      _context.set_volume(channel, f_volume);
    }
  }

  int get_sound_volume(int channel)
  {
    if (_channels[channel].sound)
    {
      return _channels[channel].volume;
    }
    return 0;
  }

  void set_global_volume(int volume)
  {
    for (int channel = 0; channel < _channels.size(); channel++)
    {
      set_sound_volume(channel, volume);
    }
  }

  void set_sound_pan(int channel, int panning)
  {
    _channels[channel].pan = panning;
  }

  clunk::Sample *get_sound(int channel) const
  {
    return _channels[channel].sound;
  }

  void stop_sound(int channel)
  {
    if (_channels[channel].sound)
    {
      _object->cancel(_channels[channel].sound->name, 0);
      _channels[channel].sound = nullptr;

      _channels[channel].priority = 0;
      _channels[channel].flags = 0;
      _channels[channel].cropos = 0;
      _channels[channel].pan = 0;
    }
  }

 private:
  static int streamCallback(
      const void *inputBuffer,
      void *outputBuffer,
      unsigned long framesPerBuffer,
      const PaStreamCallbackTimeInfo *timeInfo,
      PaStreamCallbackFlags statusFlags,
      void *userData)
  {
    if (userData == nullptr)
    {
      return 0;
    }

    auto object = static_cast<SoundManager *>(userData);
    object->_context.process(outputBuffer, framesPerBuffer * sizeof(int16_t));

    return 0;
  }

  void setupMainStream(int maxHZ)
  {
    const auto result = Pa_OpenDefaultStream(
        &_mainStream,
        0,
        2,
        paInt16,
        maxHZ,
        paFramesPerBufferUnspecified,
        streamCallback,
        this);
    if (result != paNoError)
    {
      throw std::runtime_error("Pa_OpenDefaultStream");
    }
  }

  void startStream()
  {
    const auto result = Pa_StartStream(_mainStream);
    if (result != paNoError)
    {
      throw std::runtime_error("Pa_StartStream");
    }
  }

 private:
  PaStream *_mainStream = nullptr;
  clunk::Context _context;
  clunk::Object *_object = nullptr;

  struct Channel
  {
    clunk::Sample *sound = nullptr;
    int pan = 0;
    int priority = 0;
    int cropos = 0;
    int flags = 0;
    int volume = 0;
  };
  std::vector<Channel> _channels;
};

std::unique_ptr<SoundManager> soundManager;

class WavFileReader final
{
 public:
  explicit WavFileReader(const std::string &path) : _context(avformat_alloc_context())
  {
    {
      auto input_format = av_find_input_format("wav");
      const auto result = avformat_open_input(&_context, path.c_str(), input_format, nullptr);
      if (result < 0)
      {
        throw std::runtime_error("avformat_open_input");
      }
    }

    _stream_id = get_stream_id();
    const auto codec_id = _context->streams[_stream_id]->codecpar->codec_id;
    // PCM signed 16-bit little-endian
    if (codec_id != AV_CODEC_ID_PCM_S16LE)
    {
      throw std::runtime_error("only PCM 16 bit supported");
    }
    auto codec = avcodec_find_decoder(codec_id);
    if (codec == nullptr)
    {
      throw std::runtime_error("audio codec not found");
    }

    {
      _codec_context = avcodec_alloc_context3(codec);
      if (_codec_context == nullptr)
      {
        throw std::runtime_error("avcodec_alloc_context3");
      }

      {
        const auto result = avcodec_parameters_to_context(_codec_context, _context->streams[_stream_id]->codecpar);
        if (result < 0)
        {
          throw std::runtime_error("avcodec_parameters_to_context");
        }
      }

      const auto result = avcodec_open2(_codec_context, codec, nullptr);
      if (result < 0)
      {
        throw std::runtime_error("avcodec_open2");
      }
    }
  }

  void read()
  {
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;

    auto frame = av_frame_alloc();
    if (frame == nullptr)
    {
      throw std::runtime_error("av_frame_alloc");
    }

    while (av_read_frame(_context, &packet) >= 0)
    {
      decode_packet(&packet, frame);
    }
    decode_packet(nullptr, frame);
  }

  const std::vector<uint8_t> &get_buffer() const
  {
    return _buffer;
  }

  int get_sample_rate() const
  {
    return _context->streams[_stream_id]->codecpar->sample_rate;
  }

  int get_channels() const
  {
    return _context->streams[_stream_id]->codecpar->channels;
  }

 private:
  unsigned int get_stream_id()
  {
    const auto result = avformat_find_stream_info(_context, nullptr);
    if (result < 0)
    {
      throw std::runtime_error("avformat_find_stream_info");
    }

    for (unsigned int i = 0; i < _context->nb_streams; i++)
    {
      if (_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
      {
        return i;
      }
    }

    throw std::runtime_error("audio stream not found");
  }

  void decode_packet(AVPacket *packet, AVFrame *frame)
  {
    const auto result = avcodec_send_packet(_codec_context, packet);
    if (result < 0)
    {
      throw std::runtime_error("avcodec_send_packet");
    }

    while (true)
    {
      const auto received = avcodec_receive_frame(_codec_context, frame);
      if (received == AVERROR(EAGAIN) || received == AVERROR_EOF)
      {
        return;
      }
      else if (received < 0)
      {
        throw std::runtime_error("avcodec_receive_frame");
      }

      const auto size = frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format));
      _buffer.insert(_buffer.end(), frame->extended_data[0], frame->extended_data[0] + size);
    }
  }

 private:
  AVFormatContext *_context = nullptr;
  AVCodecContext *_codec_context = nullptr;
  unsigned int _stream_id = 0;
  std::vector<uint8_t> _buffer;
};

int SoundInit(int maxHZ, int digMode, int channels)
{
  try
  {
    soundManager = std::make_unique<SoundManager>(maxHZ, digMode, channels);
    return 1;
  }
  catch (const std::exception &e)
  {
    return 0;
  }
}

void SoundPlay(void *lpDSB, int channel, int priority, int cropos, int flags)
{
  if (soundManager)
  {
    auto sample = static_cast<clunk::Sample *>(lpDSB);
    soundManager->play(sample, channel, priority, cropos, flags);
  }
}

void SoundRelease(void *lpDSB)
{
  if (soundManager)
  {
    auto sample = static_cast<clunk::Sample *>(lpDSB);
    delete sample;
  }
}

void SoundStop(int channel)
{
  if (soundManager)
  {
    soundManager->stop_sound(channel);
  }
}

void* GetSound(int channel)
{
  return soundManager ? soundManager->get_sound(channel) : nullptr;
}

void SoundLoad(char *filename, void **lpDSB)
{
  assert(soundManager != nullptr);

  try
  {
    const auto path = file::normalize_path(filename);
    WavFileReader reader(path);
    reader.read();

    clunk::Buffer buffer;
    buffer.set_data(reader.get_buffer().data(), reader.get_buffer().size());

    auto sample = soundManager->get_context().create_sample();
    sample->name = filename;
    const clunk::AudioSpec format{clunk::AudioSpec::Format::S16, reader.get_sample_rate(), static_cast<uint8_t>(reader.get_channels())};
    sample->init(buffer, format);

    *lpDSB = static_cast<void *>(sample);
  }
  catch (const std::exception &e)
  {
    printf("ERROR: %s\n", e.what());
  }
}

void SoundFinit()
{
  soundManager = nullptr;
}

void SoundVolume(int channel, int volume)
{
  if (soundManager)
  {
    soundManager->set_sound_volume(channel, volume);
  }
}

void SetVolume(void *lpDSB, int volume)
{
  assert(soundManager != nullptr);

  auto sample = static_cast<clunk::Sample *>(lpDSB);
  sample->gain = static_cast<float>(volume + 10000) * 0.0001f;
}

int GetVolume(void *lpDSB)
{
  if (soundManager)
  {
    auto sample = static_cast<clunk::Sample *>(lpDSB);
    return static_cast<int>(sample->gain * 10000.0f - 10000.0f);
  }
  return 0;
}

int GetSoundVolume(int channel)
{
  if (soundManager)
  {
    return soundManager->get_sound_volume(channel);
  }
  return 0;
}

void GlobalVolume(int volume)
{
  if (soundManager)
  {
    soundManager->set_global_volume(volume);
  }
}

void SoundPan(int channel, int panning)
{
  if (soundManager)
  {
    soundManager->set_sound_pan(channel, panning);
  }
}

int SoundStatus(int channel)
{
  return 0;
}

int SoundStatus(void* lpDSB)
{
  return 0;
}

int ChannelStatus(int channel)
{
  return 0;
}

int GetSoundFrequency(void *lpDSB)
{
  return 0;
}

void SetSoundFrequency(void *lpDSB,int frq)
{
}

void SoundStreamOpen(char *filename, void **strptr)
{
}

void SoundStreamClose(void *stream)
{
}

void SoundStreamRelease(void *stream)
{
}

void xsInitCD()
{
}

void xsMixerOpen()
{
}
