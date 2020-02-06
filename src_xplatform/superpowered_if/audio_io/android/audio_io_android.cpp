#include <SLES/OpenSLES.h>
#include <cstring>
#include "audio_io_android.hpp"
#include "utils/platform_log.h"
#include "SLES/OpenSLES_AndroidConfiguration.h"
#include "utils/helper_macros.h"
#include <math.h>

LOG_MODNAME("audio_io_android.cpp");

AndroidAudioIO::AndroidAudioIO(
      int samplerate,
      int buffersize,
      bool enableInput,
      bool enableOutput,
      StreamingAudioCallbackFn callback,
      void *clientdata,
      int inputStreamType,
      int outputStreamType)
: AudioIO()
, pInst( nullptr )
, mpExternCb(callback)
, mpExternData(clientdata)

{
  LOG_TRACE(("%s:AndroidAudioIO()\r\n", dbgModId));

  static const auto cb = [](void *clientdata, int16_t *buf16, int numberOfFrames, int samplerate) {
    AndroidAudioIO *pThis = (AndroidAudioIO *) clientdata;
    bool ok = false;
    const size_t numBytes = sizeof(float)*numberOfFrames*2;
    if (pThis->mpExternCb) {
      float * pcm = (float *)pThis->mTmp.u8DataPtr(numBytes, numBytes);
      const float div = 1.0f/32768.0f;
      const float mul = 32767.0f;
      for (int i = 0; i < numberOfFrames*2; i++){
        pcm[i] = div*buf16[i];
      }
      ok = pThis->mpExternCb(pThis->mpExternData, pcm, numberOfFrames, samplerate);
      for (int i = 0; i < numberOfFrames*2; i++){
        buf16[i] = (int)roundf(pcm[i]*mul);
      }
    }
    if (!ok){
      memset(buf16, 0, sizeof(int16_t)*numberOfFrames*2);
    }
    return ok;
  };

  pInst = new SuperpoweredAndroidAudioIO(
      samplerate,
      buffersize,
      enableInput,
      enableOutput,
      cb,
      (void *)this,
      SL_ANDROID_RECORDING_PRESET_GENERIC,
      SL_ANDROID_STREAM_MEDIA );

  (void)inputStreamType;
  (void)outputStreamType;

  LOG_ASSERT(pInst);
}

AndroidAudioIO::~AndroidAudioIO() {
  LOG_TRACE(("%s:~AndroidAudioIO()\r\n", dbgModId));
  LOG_ASSERT_WARN(pInst);
  if (pInst) {
    delete pInst;
  }
  pInst = nullptr;
}

/// @brief Call this in the main activity's onResume() method.
/// Calling this is important if you'd like to save battery. When there is no audio playing and the app goes to the background, it will automatically stop audio input and/or output.
void AndroidAudioIO::onForeground() {
  LOG_TRACE(("%s:onForeground()\r\n", dbgModId));
  LOG_ASSERT_WARN(pInst);
  if (pInst) {
    pInst->onForeground();
  }
}

/// @brief Call this in the main activity's onPause() method.
/// Calling this is important if you'd like to save battery. When there is no audio playing and the app goes to the background, it will automatically stop audio input and/or output.
void AndroidAudioIO::onBackground() {
  LOG_TRACE(("%s:onBackground()\r\n", dbgModId));
  LOG_ASSERT_WARN(pInst);
  if (pInst) {
    pInst->onBackground();
  }
}

/// @brief Starts audio input and/or output.
void AndroidAudioIO::start() {
  LOG_TRACE(("%s:start()\r\n", dbgModId));
  LOG_ASSERT_WARN(pInst);
  if (pInst) {
    pInst->start();
  }
}

/// @brief Stops audio input and/or output.
void AndroidAudioIO::stop() {
  LOG_TRACE(("%s:stop()\r\n", dbgModId));
  LOG_ASSERT_WARN(pInst);
  if (pInst) {
    pInst->stop();
  }
}

// Creates a new AudioIO class.
AudioIO *AudioIO::createNew(
    int samplerate,
    int buffersize,
    bool enableInput,
    bool enableOutput,
    StreamingAudioCallbackFn callback,
    void *clientdata,
    int inputStreamType,
    int outputStreamType
){
  LOG_TRACE(("AudioIO:createNew()\r\n"));

  AudioIO *p = new AndroidAudioIO(
      samplerate,
      buffersize,
      enableInput,
      enableOutput,
      callback,
      clientdata,
      inputStreamType,
      outputStreamType);
  LOG_ASSERT(p);

  return p;

}

