#include <PWMAudio.h>
#include "hum.h"
#include "hum_low.h"
#include "hum_high.h"
#include "lock.h"
#include "startup.h"
#include "shutdown.h"

//Sound pin
#define SOUND_PIN 7

//Create sound samples pointers.
const int16_t *hum_start = (const int16_t *)hum_wav;
const int16_t *hum_p = hum_start;

const int16_t *hum_low_start = (const int16_t *)hum_low_wav;
const int16_t *hum_low_p = hum_low_start;

const int16_t *hum_high_start = (const int16_t *)hum_high_wav;
const int16_t *hum_high_p = hum_high_start;

const int16_t *lock_start = (const int16_t *)lock_wav;
const int16_t *lock_p = lock_start;

const int16_t *startup_start = (const int16_t *)startup_wav;
const int16_t *startup_p = startup_start;

const int16_t *shutdown_start = (const int16_t *)shutdown_wav;
const int16_t *shutdown_p = shutdown_start;

bool startup_player = false;
bool shutdown_player = false;

float normal_note = 0.0;
float low_note = 0.0;
float high_note = 0.0;
float lock_note = 0.0;

unsigned int count = 0;
//Create a sound object
PWMAudio pwm(SOUND_PIN);

void audio_worker() 
{
  while(pwm.availableForWrite()) 
  {
    if(startup_player)
    {
      pwm.write(int16_t(*startup_p++));
      count += 2;
      if (count >= startup_wav_size) {
        count = 0;
        startup_p = startup_start;
        startup_player = false;
      }
    }
    else if (shutdown_player)
    {
      pwm.write(int16_t(*shutdown_p++));
      count += 2;
      if (count >= shutdown_wav_size) {
        count = 0;
        shutdown_p = startup_start;
        shutdown_player = false;
      }
    }
    else
    {
      pwm.write(int16_t((*hum_p++ * normal_note) + (*hum_low_p++ * low_note) + (*hum_high_p++ * high_note) + (*lock_p++ * lock_note)));
      count += 2;
      if (count >= hum_wav_size) {
        count = 0;
        hum_p      = hum_start;
        hum_low_p  = hum_low_start;
        hum_high_p = hum_high_start;
        lock_p = lock_start;
      }
    }
  }
}

void start_audio_worker()
{
  pwm.onTransmit(audio_worker);
  pwm.setBuffers(4, 32);
  pwm.begin(16000);
  //pwm.flush();
}
void stop_audio_worker()
{
  pwm.end();
}

void startup_sound()
{
  count = 0;
  startup_player = true;
}

void shutdown_sound()
{
  count = 0;
  shutdown_player = true;
}

int change_swing_weights(float hum, float hum_high, float hum_low, float lock)
{
  if((hum + hum_high + hum_low + lock) > 1.0)
  {
    return -1;
  }
  if(hum >= 0.0 && hum <= 1.0)
  {
    normal_note = hum;
  }
  if(hum_high >= 0.0 && hum_high <= 1.0)
  {
    high_note = hum_high;
  }
  if(hum_low >= 0.0 && hum_low <= 1.0)
  {
    low_note = hum_low;
  }
  if(lock >= 0.0 && lock <= 1.0)
  {
    lock_note = lock;
  }
  return 0;
}