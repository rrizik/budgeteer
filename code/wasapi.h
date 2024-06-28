#ifndef WASAPI_H
#define WASAPI_H

#pragma comment(lib, "ole32")
#include <mmdeviceapi.h>
#include <audioclient.h>

typedef struct WaveCursor{
    u32 id;
    u32 at;
    f32 volume;
    bool loop;
} WaveCursor;

#define WAVE_CURSORS_COUNT_MAX KB(1)
global WaveCursor wave_cursors[WAVE_CURSORS_COUNT_MAX];
global s32  wave_cursors_count = 0;

static IMMDeviceEnumerator* device_enumerator;
static IMMDevice* audio_device;
static IAudioClient* audio_client;
static IAudioRenderClient *render_client;
static WAVEFORMATEX wave_format;

static u32 buffer_samples;
REFERENCE_TIME default_device_period;
REFERENCE_TIME minimum_device_period;

static HRESULT audio_init(u16 channels, u32 samples_per_sec, u16 bits_per_sample);
static HRESULT audio_start();
static HRESULT audio_stop();
//static HRESULT audio_play_sine(f32 freq);
static HRESULT audio_play_cursors();
static bool audio_play(u32 id, f32 volume, bool loop);
static void audio_release();

#endif

