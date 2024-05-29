#include "audio.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
audio_state state = PLAY;

#define WAV_HEADER_SIZE 44

int init_audio(audio_device *ad) {
  FILE *fp = fopen("beep.wav", "rb");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open beep.wav file.\n");
    return -1;
  }
  char buffer[WAV_HEADER_SIZE];
  fread(buffer, 1, WAV_HEADER_SIZE, fp);
  unsigned short channels = *(unsigned short *)(buffer + 22);
  unsigned int freq = *(unsigned int *)(buffer + 24);
  int size = *(int *)(buffer + 40);
  ad->data = malloc(size);
  ad->size = size;
  snd_pcm_hw_params_t *hw_params;
  int err;
  if ((err = snd_pcm_open(&ad->playback_handle, "default",
                          SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    fprintf(stderr, "Cannot open default audio device.\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
    fprintf(stderr, "Cannot allocate audio device parameter struct.\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params_any(ad->playback_handle, hw_params)) < 0) {
    fprintf(stderr, "Cannot initialize audio device parameter struct.\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params_set_access(ad->playback_handle, hw_params,
                                          SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf(stderr, "Cannot set access type for hardware\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params_set_format(ad->playback_handle, hw_params,
                                          SND_PCM_FORMAT_S16_LE)) < 0) {
    fprintf(stderr, "Cannot set audio sample format.\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params_set_rate_near(ad->playback_handle, hw_params,
                                             &freq, 0)) < 0) {
    fprintf(stderr, "Cannot set audio sample rate.\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params_set_channels(ad->playback_handle, hw_params,
                                            channels)) < 0) {
    fprintf(stderr, "Cannot set audio channel count.\n");
    return -1;
  }
  if ((err = snd_pcm_hw_params(ad->playback_handle, hw_params)) < 0) {
    fprintf(stderr, "Cannot set audio device structure parameters.\n");
    return -1;
  }
  snd_pcm_hw_params_free(hw_params);
  return 0;
}

void *run_audio(void *arg) {
  audio_device *ad = (audio_device *)arg;
  audio_state current_state;
  for (;;) {
    pthread_mutex_lock(&lock);
    current_state = state;
    pthread_mutex_unlock(&lock);
  }
  return 0;
}

void close_audio(audio_device *ad) {
  free(ad->data);
  snd_pcm_close(ad->playback_handle);
}
