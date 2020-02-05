#include "media_player.hpp"
#include "superpowered_if/audio_io/audio_io.hpp"

#include <SuperpoweredSimple.h>
#include <SuperpoweredCPU.h>
#include <cstring>
#include <cmath>
#include "superpowered_if/audio_io/audio_io.hpp"
#include "utils/platform_log.h"
#include "task_sched/task_sched.h"
#include "utils/helper_macros.h"

LOG_MODNAME("media_player.cpp");

// ////////////////////////////////////////////////////////////////////////////
MediaPlayer::MediaPlayer()
: mPlayer(nullptr)
, mpAudioIO(nullptr)
, mAudioMem8()
, mFs(48000)
, mNumFrames(0)
, mOpening(false)
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
void MediaPlayer::open(
    const char *path,
    Superpowered::httpRequest *customHTTPRequest,
    bool skipSilenceAtBeginning )
{
  mPlayer->open(path, customHTTPRequest, skipSilenceAtBeginning);

  waitForOpen();
}

// ////////////////////////////////////////////////////////////////////////////
void MediaPlayer::open(const char *path,
          int offset,
          int length,
          Superpowered::httpRequest *customHTTPRequest,
          bool skipSilenceAtBeginning)
{
  mPlayer->open(path, offset, length, customHTTPRequest, skipSilenceAtBeginning);

  waitForOpen();
}

// ////////////////////////////////////////////////////////////////////////////
void MediaPlayer::waitForOpenC(void *p, uint32_t) {
  ((MediaPlayer *)p)->waitForOpen();

}
// ////////////////////////////////////////////////////////////////////////////
void MediaPlayer::waitForOpen(){
  bool ok = false;
  bool failed = false;
  Superpowered::PlayerEvent evt = mPlayer->getLatestEvent();
  switch (evt) {
    case Superpowered::PlayerEvent_None:
      LOG_TRACE(("Superpowered::PlayerEvent_None\r\n"));
      break;
    case Superpowered::PlayerEvent_Opening:
      LOG_TRACE(("Superpowered::PlayerEvent_Opening\r\n"));
      break;
    case Superpowered::PlayerEvent_OpenFailed:
      LOG_TRACE(("Superpowered::PlayerEvent_OpenFailed\r\n"));
      failed = true;
      break;
    case Superpowered::PlayerEvent_Opened:
      LOG_TRACE(("Superpowered::PlayerEvent_Opened\r\n"));
      ok = true;
      break;
    case Superpowered::PlayerEvent_ConnectionLost:
      LOG_TRACE(("Superpowered::PlayerEvent_ConnectionLost\r\n"));
      break;
    case Superpowered::PlayerEvent_ProgressiveDownloadFinished:
      LOG_TRACE(("Superpowered::PlayerEvent_ProgressiveDownloadFinished\r\n"));
      break;
    default:
      LOG_TRACE(("Superpowered::Unknown\r\n"));
      break;
  }
  if (!ok){
    int errCode = mPlayer->getOpenErrorCode();
    if (!failed) {
      TaskSchedScheduleFn(TS_PRIO_APP, waitForOpenC, this, 200);
      LOG_TRACE(("Superpowered::Got error code %d. Waiting\r\n", errCode));
    }
    else {
      LOG_TRACE(("Superpowered::Got error code %d. Failing\r\n", errCode));
    }
  }
  else {
    startAudioIO();
  }
}

// ////////////////////////////////////////////////////////////////////////////
void MediaPlayer::startAudioIO() {

  mpAudioIO = AudioIO::createNew(
      mFs,                     // device native sampling rate
      mFs / 100,                 // device native buffer size
      false,                   // enableInput
      true,                    // enableOutput
      audioProcessingC,        // process callback function
      this
  );
  mPlayer->play();
  Superpowered::CPU::setSustainedPerformanceMode(true);
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
void MediaPlayer::setSpeed(const float speed){
  if (nullptr == mPlayer) return;
  if (speed > 0) {
    mPlayer->setReverse(false);
  }
  else {
    mPlayer->setReverse(true);
  }
  float absspeed = fabs(speed);
  absspeed = MAX(0.0000101, absspeed);
  mPlayer->playbackRate = absspeed;
}

// ////////////////////////////////////////////////////////////////////////////
void MediaPlayer::setDirection(const bool reverse){
  if (nullptr == mPlayer) return;
  mPlayer->setReverse(reverse);

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
    LOG_TRACE(("MediaPlayer::audioProcessingC return false\r\n"));
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
  bool rval = true;
  if ((mPlayer) && (numberOfFrames > 0) && (samplerate > 0)) {
    mFs = samplerate;
    mPlayer->outputSamplerate = (unsigned int) samplerate;

    //float playerOutput[numberOfFrames * 2] = {0};
    const size_t bufSize = sizeof(float)*numberOfFrames*2;
    float * playerOutput = (float *)mAudioMem8.u8DataPtr(bufSize, bufSize);
    LOG_ASSERT((playerOutput));
    mNumFrames += numberOfFrames;

    if (mPlayer->processStereo(playerOutput, false, (unsigned int) numberOfFrames)) {
      Superpowered::FloatToShortInt(playerOutput, audio, (unsigned int) numberOfFrames);
      rval = true;
    }
    else {
      memset(playerOutput, 0, bufSize);
    }
  }
  if (!rval){
    LOG_TRACE(("MediaPlayer::audioProcessing return false\r\n"));
  }
  return rval;
}


