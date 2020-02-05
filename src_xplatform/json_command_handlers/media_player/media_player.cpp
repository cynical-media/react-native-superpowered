#include "media_player.hpp"
#include "superpowered_if/audio_io/audio_io.hpp"

#include <SuperpoweredSimple.h>
#include <SuperpoweredCPU.h>
#include "superpowered_if/audio_io/audio_io.hpp"

// ////////////////////////////////////////////////////////////////////////////
MediaPlayer::MediaPlayer()
: mPlayer(nullptr)
, mpAudioIO(nullptr)
, mAudioMem8()
, mFs(48000)
{
  // TODO: Do in jni layer
  //  Superpowered::AdvancedAudioPlayer::setTempFolder(tempFolder);
  mPlayer = new Superpowered::AdvancedAudioPlayer(mFs, 0);

}

// ////////////////////////////////////////////////////////////////////////////
MediaPlayer::~MediaPlayer(){
  delete mPlayer;
  mPlayer = nullptr;
  delete mpAudioIO;
  mpAudioIO = nullptr;
  mAudioMem8.clear();
}

// ////////////////////////////////////////////////////////////////////////////
//bool MediaPlayer::start(){
//  mPlayer->play();
//  Superpowered::CPU::setSustainedPerformanceMode(true);
//}

// ////////////////////////////////////////////////////////////////////////////
void MediaPlayer::open(
    const char *path,
    Superpowered::httpRequest *customHTTPRequest,
    bool skipSilenceAtBeginning )
{
  mPlayer->open(path, customHTTPRequest, skipSilenceAtBeginning);
  Superpowered::CPU::setSustainedPerformanceMode(true);
}

// ////////////////////////////////////////////////////////////////////////////
void MediaPlayer::open(const char *path,
          int offset,
          int length,
          Superpowered::httpRequest *customHTTPRequest,
          bool skipSilenceAtBeginning)
{
  mPlayer->open(path, offset, length, customHTTPRequest, skipSilenceAtBeginning);
  Superpowered::CPU::setSustainedPerformanceMode(true);
}

// ////////////////////////////////////////////////////////////////////////////
void MediaPlayer::startAudioIO() {
  mpAudioIO = AudioIO::createNew(
      mFs,                     // device native sampling rate
      mFs/100,                 // device native buffer size
      false,                   // enableInput
      true,                    // enableOutput
      audioProcessingC,        // process callback function
      this
  );
}

// ////////////////////////////////////////////////////////////////////////////
bool MediaPlayer::stop() {
  if (mPlayer->isPlaying()) {
    mPlayer->togglePlayback();
  }
  Superpowered::CPU::setSustainedPerformanceMode(false);
  return true;
}

// ////////////////////////////////////////////////////////////////////////////
// This is called periodically by the audio engine.
bool MediaPlayer::audioProcessingC (
    void * clientdata, // custom pointer
    short int *audio,           // output buffer
    int numberOfFrames,         // number of frames to process
    int samplerate              // current sample rate in Hz
){
  MediaPlayer *pMedia = (MediaPlayer *)clientdata;
  if (nullptr == pMedia){
    return false;
  }
  else {
    return pMedia->audioProcessing(audio, numberOfFrames, samplerate);
  }
}

// ////////////////////////////////////////////////////////////////////////////
// This is called periodically by the audio engine.
bool MediaPlayer::audioProcessing (
    short int *audio,           // output buffer
    int numberOfFrames,         // number of frames to process
    int samplerate              // current sample rate in Hz
) {
  bool rval = false;
  if (mPlayer) {
    mPlayer->outputSamplerate = (unsigned int) samplerate;
    //float playerOutput[numberOfFrames * 2];
    const size_t bufSize = sizeof(float)*numberOfFrames*2;
    float * playerOutput = (float *)mAudioMem8.u8DataPtr(bufSize, bufSize);

    if (mPlayer->processStereo(playerOutput, false, (unsigned int) numberOfFrames)) {
      Superpowered::FloatToShortInt(playerOutput, audio, (unsigned int) numberOfFrames);
      rval = true;
    }
  }
  return rval;
}


