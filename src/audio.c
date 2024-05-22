#include "audio.h"
#include <alsa/asoundlib.h>
#include <alsa/error.h>
#include <alsa/pcm.h>
#include <stdio.h>

typedef struct {
  snd_pcm_t *playback_handle;
} audio_device;

audio_device ad;

int init_audio() {
  snd_pcm_hw_params_t *hw_params;
  int err;
  if ((err = snd_pcm_open(&ad.playback_handle, "default",
                          SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    fprintf(stderr, "Cannot open default audio device.\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
    fprintf(stderr, "Cannot allocate audio device parameter struct.\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params_any(ad.playback_handle, hw_params)) < 0) {
    fprintf(stderr, "Cannot initialize audio device parameter struct.\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params_set_access(ad.playback_handle, hw_params,
                                          SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf(stderr, "Cannot set access type for hardware\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params_set_format(ad.playback_handle, hw_params,
                                          SND_PCM_FORMAT_S16_LE)) < 0) {
    fprintf(stderr, "Cannot set audio sample format.\n");
    return -1;
  }
  unsigned int freq = 0;
  if ((err = snd_pcm_hw_params_set_rate_near(ad.playback_handle, hw_params,
                                             &freq, 0)) < 0) {
    fprintf(stderr, "Cannot set audio sample rate.\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params_set_channels(ad.playback_handle, hw_params, 2)) <
      0) {
    fprintf(stderr, "Cannot set audio channel count.\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params(ad.playback_handle, hw_params)) < 0) {
    fprintf(stderr, "Cannot set audio device structure parameters.\n");
    return -1;
  }
  snd_pcm_hw_params_free(hw_params);
  return 0;
}

void close_audio() { snd_pcm_close(ad.playback_handle); }
