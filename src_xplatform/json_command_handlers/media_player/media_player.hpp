#ifndef MEDIA_PLAYER_HPP
#define MEDIA_PLAYER_HPP
#ifdef __cplusplus

#include <Superpowered.h>
#include <SuperpoweredAdvancedAudioPlayer.h>
#include "superpowered_if/audio_io/audio_io.hpp"
#include "utils/simple_string.hpp"

class MediaPlayer {
public:
  MediaPlayer();

  ~MediaPlayer();

  void open(
      const char *path,
      Superpowered::httpRequest *customHTTPRequest = 0,
      bool skipSilenceAtBeginning = false);

  void open(const char *path,
      int offset,
      int length,
      Superpowered::httpRequest *customHTTPRequest = 0,
      bool skipSilenceAtBeginning = false);

  bool stop();

private:

  static void waitForOpenC(void *p, uint32_t);
  void waitForOpen();

  static bool audioProcessingC (
      void * clientdata, // custom pointer
      short int *audio,           // output buffer
      int numberOfFrames,         // number of frames to process
      int samplerate              // current sample rate in Hz
  );

  bool audioProcessing (
      short int *audio,           // output buffer
      int numberOfFrames,         // number of frames to process
      int samplerate              // current sample rate in Hz
  );

  void startAudioIO();

private:
  Superpowered::AdvancedAudioPlayer *mPlayer;
  AudioIO *mpAudioIO;
  sstring mAudioMem8;
  unsigned int mFs;
  uint64_t mNumFrames;
  bool mOpening;
};


#endif // #ifdef __cplusplus
#endif