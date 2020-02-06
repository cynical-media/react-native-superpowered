#ifndef AUDIO_IO_ANDROID_HPP
#define AUDIO_IO_ANDROID_HPP
#include "OpenSource/SuperpoweredAndroidAudioIO.h"
#include "superpowered_if/audio_io/audio_io.hpp"
#include "utils/simple_string.hpp"

class AndroidAudioIO : public AudioIO {
public:
  /// @brief Creates an audio I/O instance. Audio input and/or output immediately starts after calling this.
  /// @param samplerate The requested sample rate in Hz.
  /// @param buffersize The requested buffer size (number of frames).
  /// @param enableInput Enable audio input.
  /// @param enableOutput Enable audio output.
  /// @param callback The audio processing callback function to call periodically.
  /// @param clientdata A custom pointer the callback receives.
  /// @param inputStreamType OpenSL ES stream type, such as SL_ANDROID_RECORDING_PRESET_GENERIC. -1 means default. SLES/OpenSLES_AndroidConfiguration.h has them.
  /// @param outputStreamType OpenSL ES stream type, such as SL_ANDROID_STREAM_MEDIA or SL_ANDROID_STREAM_VOICE. -1 means default. SLES/OpenSLES_AndroidConfiguration.h has them.
  AndroidAudioIO(
      int samplerate,
      int buffersize,
      bool enableInput,
      bool enableOutput,
      StreamingAudioCallbackFn callback,
      void *clientdata,
      int inputStreamType = -1,
      int outputStreamType = -1);

  ~AndroidAudioIO();

/// @brief Call this in the main activity's onResume() method.
/// Calling this is important if you'd like to save battery. When there is no audio playing and the app goes to the background, it will automatically stop audio input and/or output.
  void onForeground() override;

/// @brief Call this in the main activity's onPause() method.
/// Calling this is important if you'd like to save battery. When there is no audio playing and the app goes to the background, it will automatically stop audio input and/or output.
  void onBackground() override;

/// @brief Starts audio input and/or output.
  void start() override;

/// @brief Stops audio input and/or output.
  void stop() override;

private:
  SuperpoweredAndroidAudioIO * pInst;
  sstring mTmp;
  StreamingAudioCallbackFn mpExternCb;
  void *mpExternData;
};

#endif
