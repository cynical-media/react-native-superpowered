#include "audio_io_android.hpp"

AndroidAudioIO::AndroidAudioIO(
      int samplerate,
      int buffersize,
      bool enableInput,
      bool enableOutput,
      audioProcessingCallback callback,
      void *clientdata,
      int inputStreamType,
      int outputStreamType)
: AudioIO()
, pInst( nullptr )
{
  pInst = new SuperpoweredAndroidAudioIO(
      samplerate,
      buffersize,
      enableInput,
      enableOutput,
      callback,
      clientdata,
      inputStreamType,
      outputStreamType );

}

AndroidAudioIO::~AndroidAudioIO() {
  delete pInst;
}

/// @brief Call this in the main activity's onResume() method.
/// Calling this is important if you'd like to save battery. When there is no audio playing and the app goes to the background, it will automatically stop audio input and/or output.
void AndroidAudioIO::onForeground() {
  pInst->onForeground();
}

/// @brief Call this in the main activity's onPause() method.
/// Calling this is important if you'd like to save battery. When there is no audio playing and the app goes to the background, it will automatically stop audio input and/or output.
void AndroidAudioIO::onBackground() {
  pInst->onBackground();
}

/// @brief Starts audio input and/or output.
void AndroidAudioIO::start() {
  pInst->start();
}

/// @brief Stops audio input and/or output.
void AndroidAudioIO::stop() {
  pInst->stop();
}

// Creates a new AudioIO class.
AudioIO *AudioIO::createNew(
    int samplerate,
    int buffersize,
    bool enableInput,
    bool enableOutput,
    audioProcessingCallback callback,
    void *clientdata,
    int inputStreamType,
    int outputStreamType
){
  return new AndroidAudioIO(
      samplerate,
      buffersize,
      enableInput,
      enableOutput,
      callback,
      clientdata,
      inputStreamType,
      outputStreamType);

}

