///////////////////////////////////////////////////////////////////////////////
// sts_mixer.h - v0.02
// written 2016 by Sebastian Steinhauer
//
//  LICENSE
//    Public domain. See "unlicense" statement at the end of this file.
//
//  ABOUT
//    A simple stereo audio mixer which is capable of mixing samples and audio streams.
//    Samples can be played with different gain, pitch and panning.
//    Streams can be played with different gain.
//    This library has no malloc/free. All structs have to be "prepared" by the user. So you can enroll your own memory management.
//    You have to implement/provide a real audio-backend to hear something from the speakers.
//    A good starting point would be SDL2 where you can use an audio callback to feed the audio device.
//
//  USAGE
//    Please note that most audio systems will run in a separate thread. So you have to take care about locking before modifying the sts_mixer_t state.
//    See the example at the end of the file.
//
//  VERSION HISTORY
//    0.03 (2022-12-12) add an ability to stop audio stream via callback, add a method to check if voice is already stopped
//    0.02 (2022-05-10) allow voice queueing in same channel. ie, chain another sample on same voice channel after current sample playback is done (@r-lyeh)
//    0.01 (2016-05-01) initial version
//
#ifndef __INCLUDED__STS_MIXER_H__
#define __INCLUDED__STS_MIXER_H__


// The number of concurrent voices (channels) which are used to mix the audio.
// If you need more, use a higher number by setting #define STS_MIXER_VOICE n before including this header.
#ifndef STS_MIXER_VOICES
#define STS_MIXER_VOICES      32
#endif // STS_MIXER_VOICES

// Defines the various audio formats. Note that they are all on system endianess.
enum {
  STS_MIXER_SAMPLE_FORMAT_NONE,               // no format
  STS_MIXER_SAMPLE_FORMAT_8,                  // signed 8-bit
  STS_MIXER_SAMPLE_FORMAT_16,                 // signed 16-bit
  STS_MIXER_SAMPLE_FORMAT_32,                 // signed 32-bit
  STS_MIXER_SAMPLE_FORMAT_FLOAT               // floats
};


////////////////////////////////////////////////////////////////////////////////
//
//  SAMPLES
//
// A sample is a *MONO* piece of audio which is loaded fully to memory.
// It can be played with various gains, pitches and pannings.
//
typedef struct {
  unsigned int              length;           // length in samples (so 1024 samples of STS_MIXER_SAMPLE_FORMAT_16 would be 2048 bytes)
  unsigned int              frequency;        // frequency of this sample (e.g. 44100, 22000 ...)
  int                       audio_format;     // one of STS_MIXER_SAMPLE_FORMAT_*
  void*                     data;             // pointer to the sample data, sts_mixer makes no copy, so you have to keep them in memory
  void*                     next;             // next sample in chain (if any) //< @r-lyeh
} sts_mixer_sample_t;


////////////////////////////////////////////////////////////////////////////////
//
//  STREAMS
//
// A stream is *STEREO* audio which will be decoded/loaded as needed.
// It can be played with various gains. No panning or pitching.
//

// The callback which will be called when the stream needs more data.
typedef bool (*sts_mixer_stream_callback)(sts_mixer_sample_t* sample, void* userdata);

typedef struct {
  void*                     userdata;         // a userdata pointer which will passed to the callback
  sts_mixer_stream_callback callback;         // this callback will be called when the stream needs more data
  sts_mixer_sample_t        sample;           // the current stream "sample" which holds the current piece of audio
} sts_mixer_stream_t;


////////////////////////////////////////////////////////////////////////////////
//
//  VOICES
//
// A voice is an audio source which will be used during mixing.
// It can play nothing, a sample or a stream.
// Most of those fields are considered "private" and you should not play around with those.
//
typedef struct {
  int                       state;
  sts_mixer_sample_t*       sample;
  sts_mixer_stream_t*       stream;
  float                     position;
  float                     gain;
  float                     pitch;
  float                     pan;
} sts_mixer_voice_t;


////////////////////////////////////////////////////////////////////////////////
//
//  MIXER
//
// The mixer state.
//
typedef struct {
  float                     gain;             // the global gain (you can change it if you want to change to overall volume)
  unsigned int              frequency;        // the frequency for the output of mixed audio data
  int                       audio_format;     // the audio format for the output of mixed audio data
  sts_mixer_voice_t         voices[STS_MIXER_VOICES]; // holding all audio voices for this state
} sts_mixer_t;


////////////////////////////////////////////////////////////////////////////////
//
//  API
//

// "Initializes" a new sts_mixer state.
void sts_mixer_init(sts_mixer_t* mixer, unsigned int frequency, int audio_format);
// "Shutdown" the mixer state. It will simply reset all fields.
void sts_mixer_shutdown(sts_mixer_t* mixer);

// Return the number of active voices. Active voices are voices that play either a stream or a sample.
int sts_mixer_get_active_voices(sts_mixer_t* mixer);

// Play the given sample with the gain, pitch and panning.
// Panning can be something between -1.0f (fully left) ...  +1.0f (fully right)
// Please note that pitch will be clamped so it cannot reach 0.0f (would be useless).
// Returns the number of the voice where this sample will be played or -1 if no voice was free.
int sts_mixer_play_sample(sts_mixer_t* mixer, sts_mixer_sample_t* sample, float gain, float pitch, float pan);

// Plays the given stream with the gain.
// Returns the number of the voice where this stream will be played or -1 if no voice was free.
int sts_mixer_play_stream(sts_mixer_t* mixer, sts_mixer_stream_t* stream, float gain);

// Stops voice with the given voice no. You can pass the returned number of sts_mixer_play_sample / sts_mixer_play_stream here.
void sts_mixer_stop_voice(sts_mixer_t* mixer, int voice);

// Returns whether the given sample has already stopped playing.
bool sts_mixer_sample_stopped(sts_mixer_t* mixer, sts_mixer_sample_t* sample);

// Returns whether the given stream has already stopped playing.
bool sts_mixer_stream_stopped(sts_mixer_t* mixer, sts_mixer_stream_t* stream);

// Stops all voices playing the given sample. Useful when you want to delete the sample and make sure it is not used anymore.
void sts_mixer_stop_sample(sts_mixer_t* mixer, sts_mixer_sample_t* sample);

// Stops all voices playing the given stream. Useful when you want to delete the stream and make sure it is not used anymore.
void sts_mixer_stop_stream(sts_mixer_t* mixer, sts_mixer_stream_t* stream);

// The mixing function. You should call the function if you need to pass more audio data to the audio device.
// Typically this function is called in a separate thread or something like that.
// It will write audio data in the specified format and frequency of the mixer state.
void sts_mixer_mix_audio(sts_mixer_t* mixer, void* output, unsigned int samples);


#endif // __INCLUDED__STS_MIXER_H__

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
////    IMPLEMENTATION
////
////
#ifdef STS_MIXER_IMPLEMENTATION

enum {
  STS_MIXER_VOICE_STOPPED,
  STS_MIXER_VOICE_PLAYING,
  STS_MIXER_VOICE_STREAMING
};


static float sts_mixer__clamp(const float value, const float min, const float max) {
  if (value < min) return min;
  else if (value > max) return max;
  else return value;
}


static float sts_mixer__clamp_sample(const float sample) {
  if (sample < -1.0f) return -1.0f;
  else if (sample > 1.0f) return 1.0f;
  else return sample;
}


static float sts_mixer__get_sample(sts_mixer_sample_t* sample, unsigned int position) {
  switch (sample->audio_format) {
    case STS_MIXER_SAMPLE_FORMAT_8:
      return (float)((char*)sample->data)[position] / 127.0f;
    case STS_MIXER_SAMPLE_FORMAT_16:
      return (float)((short*)sample->data)[position] / 32767.0f;
    case STS_MIXER_SAMPLE_FORMAT_32:
      return (float)((int*)sample->data)[position] / 2147483647.0f;
    case STS_MIXER_SAMPLE_FORMAT_FLOAT:
      return ((float*)sample->data)[position];
    default:
      return 0.0f;
  }
}


static void sts_mixer__reset_voice(sts_mixer_t* mixer, const int i) {
  sts_mixer_voice_t*  voice = &mixer->voices[i];
  voice->state = STS_MIXER_VOICE_STOPPED;
  voice->sample = 0;
  voice->stream = 0;
  voice->position = voice->gain = voice->pitch = voice->pan = 0.0f;
}


static int sts_mixer__find_free_voice(sts_mixer_t* mixer) {
  int i;

  for (i = 0; i < STS_MIXER_VOICES; ++i) {
    if (mixer->voices[i].state == STS_MIXER_VOICE_STOPPED) return i;
  }
  return -1;
}


void sts_mixer_init(sts_mixer_t* mixer, unsigned int frequency, int audio_format) {
  int i;

  for (i = 0; i < STS_MIXER_VOICES; ++i) sts_mixer__reset_voice(mixer, i);
  mixer->frequency = frequency;
  mixer->gain = 1.0f;
  mixer->audio_format = audio_format;
}


void sts_mixer_shutdown(sts_mixer_t* mixer) {
  sts_mixer_init(mixer, 0, 0);
}


int sts_mixer_get_active_voices(sts_mixer_t* mixer) {
  int   i, active;
  for (i = 0, active = 0; i < STS_MIXER_VOICES; ++i) {
    if (mixer->voices[i].state != STS_MIXER_VOICE_STOPPED) ++active;
  }
  return active;
}


int sts_mixer_play_sample(sts_mixer_t* mixer, sts_mixer_sample_t* sample, float gain, float pitch, float pan) {
  int                 i;
  sts_mixer_voice_t*  voice;

  i = sts_mixer__find_free_voice(mixer);
  if (i >= 0) {
    voice = &mixer->voices[i];
    voice->gain = gain;
    voice->pitch = sts_mixer__clamp(pitch, 0.1f, 10.0f);
    voice->pan = sts_mixer__clamp(pan * 0.5f, -0.5f, 0.5f);
    voice->position = 0.0f;
    voice->sample = sample;
    voice->stream = 0;
    voice->state = STS_MIXER_VOICE_PLAYING;
  }
  return i;
}


int sts_mixer_play_stream(sts_mixer_t* mixer, sts_mixer_stream_t* stream, float gain) {
  int                 i;
  sts_mixer_voice_t*  voice;

  i = sts_mixer__find_free_voice(mixer);
  if (i >= 0) {
    voice = &mixer->voices[i];
    voice->gain = gain;
    voice->position = 0.0f;
    voice->sample = 0;
    voice->stream = stream;
    voice->state = STS_MIXER_VOICE_STREAMING;
  }
  return i;
}


void sts_mixer_stop_voice(sts_mixer_t* mixer, int voice) {
  if (voice >= 0 && voice < STS_MIXER_VOICES) sts_mixer__reset_voice(mixer, voice);
}

bool sts_mixer_sample_stopped(sts_mixer_t* mixer, sts_mixer_sample_t* sample) {
  for (int i = 0; i < STS_MIXER_VOICES; ++i) {
    if (mixer->voices[i].sample == sample && mixer->voices[i].state != STS_MIXER_VOICE_STOPPED) return false;
  }
  return true;
}

bool sts_mixer_stream_stopped(sts_mixer_t* mixer, sts_mixer_stream_t* stream) {
  for (int i = 0; i < STS_MIXER_VOICES; ++i) {
    if (mixer->voices[i].stream == stream && mixer->voices[i].state != STS_MIXER_VOICE_STOPPED) return false;
  }
  return true;
}

void sts_mixer_stop_sample(sts_mixer_t* mixer, sts_mixer_sample_t* sample) {
  int i;

  for (i = 0; i < STS_MIXER_VOICES; ++i) {
    if (mixer->voices[i].sample == sample) sts_mixer__reset_voice(mixer, i);
  }
}


void sts_mixer_stop_stream(sts_mixer_t* mixer, sts_mixer_stream_t* stream) {
  int i;

  for (i = 0; i < STS_MIXER_VOICES; ++i) {
    if (mixer->voices[i].stream == stream) sts_mixer__reset_voice(mixer, i);
  }
}


void sts_mixer_mix_audio(sts_mixer_t* mixer, void* output, unsigned int samples) {
  sts_mixer_voice_t*  voice;
  unsigned int        i, position;
  float               left, right, advance, sample;
  char*               out_8 = (char*)output;
  short*              out_16 = (short*)output;
  int*                out_32 = (int*)output;
  float*              out_float = (float*)output;

  // mix all voices
  advance = 1.0f / (float)mixer->frequency;
  for (; samples > 0; --samples) {
    left = right = 0.0f;
    for (i = 0; i < STS_MIXER_VOICES; ++i) {
      voice = &mixer->voices[i];
      if (voice->state == STS_MIXER_VOICE_PLAYING) {
        position = (int)voice->position;
        if (position < voice->sample->length) {
          sample = sts_mixer__clamp_sample(sts_mixer__get_sample(voice->sample, position) * voice->gain);
          left += sts_mixer__clamp_sample(sample * (0.5f - voice->pan));
          right += sts_mixer__clamp_sample(sample * (0.5f + voice->pan));
          voice->position += (float)voice->sample->frequency * advance * voice->pitch;
        } else if( voice->sample->next ) { //< @r-lyeh
          *voice->sample = *(sts_mixer_sample_t*)voice->sample->next; //< @r-lyeh
          voice->position = 0; //< @r-lyeh
        } else sts_mixer__reset_voice(mixer, i);
      } else if (voice->state == STS_MIXER_VOICE_STREAMING) {
        position = ((int)voice->position) * 2;
        if (position >= voice->stream->sample.length) {
          // buffer empty...refill
          if (voice->stream->callback(&voice->stream->sample, voice->stream->userdata)) {
            voice->position = 0.0f;
            position = 0;
          } else {
            sts_mixer__reset_voice(mixer, i);
            continue;
          }
        }
        left += sts_mixer__clamp_sample(sts_mixer__get_sample(&voice->stream->sample, position) * voice->gain);
        right += sts_mixer__clamp_sample(sts_mixer__get_sample(&voice->stream->sample, position + 1) * voice->gain);
        voice->position += (float)voice->stream->sample.frequency * advance;
      }
    }

    // write to buffer.
    float _g = mixer->gain; //< @r-lyeh: added master gain
    float _127 = 127.0f * _g; //< @r-lyeh: added master gain
    float _32767 = 32767.0f * _g; //< @r-lyeh: added master gain
    float _2147483647 = 2147483647.0f * _g; //< @r-lyeh: added master gain
    left = sts_mixer__clamp_sample(left);
    right = sts_mixer__clamp_sample(right);
    switch (mixer->audio_format) {
      case STS_MIXER_SAMPLE_FORMAT_8:
        *out_8++ = (char)(left * _127); //< @r-lyeh: added master gain
        *out_8++ = (char)(right * _127); //< @r-lyeh: added master gain
        break;
      case STS_MIXER_SAMPLE_FORMAT_16:
        *out_16++ = (short)(left * _32767); //< @r-lyeh: added master gain
        *out_16++ = (short)(right * _32767); //< @r-lyeh: added master gain
        break;
      case STS_MIXER_SAMPLE_FORMAT_32:
        *out_32++ = (int)(left * _2147483647); //< @r-lyeh: added master gain
        *out_32++ = (int)(right * _2147483647); //< @r-lyeh: added master gain
        break;
      case STS_MIXER_SAMPLE_FORMAT_FLOAT:
        *out_float++ = left * _g; //< @r-lyeh: added master gain
        *out_float++ = right * _g; //< @r-lyeh: added master gain
        break;
    }
  }
}
#endif // STS_MIXER_IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////
//  EXAMPLE
//    This is a very simple example loading a stream and a sample using
//    dr_flac.h (https://github.com/mackron/dr_libs) and SDL2. You can of course also use stb_vorbis or something similar :)
//    Please note how the audio thread of SDL2 will be locked when the mixer state get's modified. This is important!
//    Also there's no error checking in the entire example code, so beware.
//
#if 0
#include "SDL.h"


#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"
#define STS_MIXER_IMPLEMENTATION
#include "sts_mixer.h"


SDL_AudioDeviceID   audio_device = 0;
sts_mixer_t         mixer;


// encapsulate drflac and some buffer with the sts_mixer_stream_t
typedef struct {
  drflac*             flac;               // FLAC decoder state
  sts_mixer_stream_t  stream;             // mixer stream
  int32_t             data[4096*2];       // static sample buffer
} mystream_t;


// SDL2 audio callback
static void audio_callback(void* userdata, Uint8* stream, int len) {
  (void)(userdata);
  sts_mixer_mix_audio(&mixer, stream, len / (sizeof(int) * 2));
}


// load a sample
static void load_sample(sts_mixer_sample_t* sample, const char *filename) {
  drflac* flac = drflac_open_file(filename);
  sample->frequency = flac->sampleRate;
  sample->audio_format = STS_MIXER_SAMPLE_FORMAT_32;
  sample->length = flac->totalSampleCount;
  sample->data = malloc(sample->length * sizeof(int32_t));
  drflac_read_s32(flac, sample->length, (int32_t*)sample->data);
  drflac_close(flac);
}


// the callback to refill the stream data
static void refill_stream(sts_mixer_sample_t* sample, void* userdata) {
  mystream_t* stream = (mystream_t*)userdata;
  if (drflac_read_s32(stream->flac, sample->length, stream->data) < sample->length) drflac_seek_to_sample(stream->flac, 0);
}


// load a stream
static void load_stream(mystream_t* stream, const char *filename) {
  stream->flac = drflac_open_file(filename);
  stream->stream.userdata = stream;
  stream->stream.callback = refill_stream;
  stream->stream.sample.frequency = stream->flac->sampleRate;
  stream->stream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_32;
  stream->stream.sample.length = 4096*2;
  stream->stream.sample.data = stream->data;
  refill_stream(&stream->stream.sample, stream);
}


// helper to get random [0.0f..1.0f values
static float randf() {
  return (float)(rand()) / (float)RAND_MAX;
}


int main(int argc, char *argv[]) {
  SDL_AudioSpec       want, have;
  sts_mixer_sample_t  sample;
  mystream_t          stream;


  (void)(argc); (void)(argv);

  // init SDL2 + audio
  want.format = AUDIO_S32SYS;
  want.freq = 44100;
  want.channels = 2;
  want.userdata = NULL;
  want.samples = 4096;
  want.callback = audio_callback;
  SDL_Init(SDL_INIT_AUDIO);
  audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

  // init sts_mixer and load things
  sts_mixer_init(&mixer, 44100, STS_MIXER_SAMPLE_FORMAT_32);
  load_sample(&sample, "effect.flac");
  load_stream(&stream, "music.flac");

  // play the stream
  sts_mixer_play_stream(&mixer, &stream.stream, 0.7f);

  // start audio processing and do a loop for audio effects
  SDL_PauseAudioDevice(audio_device, 0);
  for (;;) {
    // !!!IMPORTANT!!! lock the audio thread before modifying data in the sts_mixer !!!
    SDL_LockAudioDevice(audio_device);
    // play a sample with random gain, pitch and panning
    sts_mixer_play_sample(&mixer, &sample, randf(), 0.5f + randf(), -1.0f + randf() * 2.0f);
    // unlock audio thread again
    SDL_UnlockAudioDevice(audio_device);

    // wait ...
    SDL_Delay(76);
  }
  SDL_PauseAudioDevice(audio_device, 1);
  SDL_CloseAudioDevice(audio_device);

  SDL_Quit();
  return 0;
}
#endif // 0
/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org/>
*/
