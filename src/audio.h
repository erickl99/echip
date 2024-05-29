#ifndef audio_h
#define audio_h

#include <alsa/asoundlib.h>
#include <pthread.h>

typedef struct {
  snd_pcm_t *playback_handle;
  size_t size;
  char *data;
} audio_device;

typedef enum {
  PLAY,
  PAUSE,
  END,
} audio_state;

int init_audio(audio_device *ad);
void *run_audio(void *arg);
void close_audio(audio_device *ad);
#endif // !audio_h
