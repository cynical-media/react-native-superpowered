#import "audio_io.hpp"
#import "audio_io_ios.hpp"
#import "SuperpoweredIOSAudioIO.h"
#include "Superpowered.h"
#include "SuperpoweredAdvancedAudioPlayer.h"
#include "SuperpoweredSimple.h"
#include "utils/simple_string.hpp"
#include "utils/platform_log.h"

LOG_MODNAME("ios_audio");

@interface AudioIO_IOS_objc: NSObject <AVAudioPlayerDelegate, AVAudioRecorderDelegate> {
}
//@property (nonatomic, strong, retain) CBCentralManager *cBCM;
//@property (strong, nonatomic, retain) NSMutableArray *peripheralsAry;
//@property (weak, nonatomic, getter=getApplication) UIApplication *application;
@end

class AudioIO_IOS: public AudioIO {
public:
  AudioIO_IOS(
    int samplerate,
    int buffersize,
    bool enableInput,
    bool enableOutput,
    audioProcessingCallback callback,
    void *clientdata,
    int inputStreamType = -1,
    int outputStreamType = -1);
  
   virtual ~AudioIO_IOS();

  /**
   @brief Call this in the main activity's onResume() method.
   Calling this is important if you'd like to save battery. When there is no audio playing and the app goes to the background, it will automatically stop audio input and/or output.
   */
  void onForeground() override;

  /**
   @brief Call this in the main activity's onPause() method.
   Calling this is important if you'd like to save battery. When there is no audio playing and the app goes to the background, it will automatically stop audio input and/or output.
   */
  void onBackground() override;

   ///@brief Starts audio input and/or output.
  void start() override;

   /// @brief Stops audio input and/or output.
  void stop() override;
  
protected:
  
  static bool audioProcessingCallbackCbC(
    void *clientData,
    float **inputBuffers,
    unsigned int inputChannels,
    float **outputBuffers,
    unsigned int outputChannels,
    unsigned int numberOfFrames,
    unsigned int samplerate,
    unsigned long long hostTime);
  
 bool audioProcessingCallbackCb(
    float **inputBuffers,
    unsigned int inputChannels,
    float **outputBuffers,
    unsigned int outputChannels,
    unsigned int numberOfFrames,
    unsigned int samplerate,
    unsigned long long hostTime);
  
  AudioIO_IOS_objc *mpAudio;
  audioProcessingCallback const mCallbackFn;
  void * const mpClientData;
  sstring mAudioBuf;


};

@implementation AudioIO_IOS_objc {
  SuperpoweredIOSAudioIO *_audioIO;
  AudioIO_IOS *_audioIOClass;
  void *_userClientData;
  audioProcessingCallback _userCallbackFn;
    //Superpowered::AdvancedAudioPlayer *player;
}

- (id)initWithClass: (AudioIO_IOS *)pClass
            preferredBufferSize:(unsigned int)prefBufferSize
            preferredSamplerate:(unsigned int)prefsamplerate
            channels:(int)channels
            audioProcessingCallback:(audioProcessingCallback)callback
            clientdata:(void *)clientdata
{
  self = [super init];
  
  static const auto cb = [](void *clientData, float **inputBuffers, unsigned int inputChannels, float **outputBuffers, unsigned int outputChannels, unsigned int numberOfFrames, unsigned int samplerate, unsigned long long hostTime){
    AudioIO_IOS_objc *pSelf = (__bridge AudioIO_IOS_objc *)clientData;
    bool rval = pSelf->_userCallbackFn(
      pSelf->_userClientData,
      inputBuffers,
      inputChannels,
      outputBuffers,
      outputChannels,
      numberOfFrames,
      samplerate,
      hostTime);
    return rval;
  };
  
  if (self) {
    _audioIOClass = pClass;
    _userClientData = clientdata;
    _userCallbackFn = callback;

    _audioIO = [[SuperpoweredIOSAudioIO alloc]
    initWithDelegate:(id<SuperpoweredIOSAudioIODelegate>)self
    preferredBufferSize:12
    preferredSamplerate:44100
    audioSessionCategory:AVAudioSessionCategoryPlayback
    channels:2
    audioProcessingCallback:cb
    clientdata:(__bridge void *)self];
    
    [_audioIO start];

  }
  return self;
}

- (void)dealloc {
#if !__has_feature(objc_arc)
    [super dealloc];
#endif
}

-(AudioIO_IOS *)getClass {
  return _audioIOClass;
}
@end

AudioIO_IOS::AudioIO_IOS(
    int samplerate,
    int buffersize,
    bool enableInput,
    bool enableOutput,
    audioProcessingCallback callback,
    void *clientdata,
    int inputStreamType,
    int outputStreamType)
: mpAudio(nullptr)
, mCallbackFn(callback)
, mpClientData(clientdata)
{

  // Pointer to the objective C interface.
  mpAudio = [[AudioIO_IOS_objc alloc]
                              initWithClass: this
                              preferredBufferSize:buffersize
                              preferredSamplerate:samplerate
                              channels:2
                              audioProcessingCallback:audioProcessingCallbackCbC
                              clientdata:this];
    
}

bool AudioIO_IOS::audioProcessingCallbackCbC(
  void *clientData,
  float **inputBuffers,
  unsigned int inputChannels,
  float **outputBuffers,
  unsigned int outputChannels,
  unsigned int numberOfFrames,
  unsigned int samplerate,
  unsigned long long hostTime){
  
  AudioIO_IOS *pThis = (AudioIO_IOS *)clientData;
  bool rval = false;
  if (pThis){
    rval = pThis->audioProcessingCallbackCb(
      inputBuffers,
      inputChannels,
      outputBuffers,
      outputChannels,
      numberOfFrames,
      samplerate,
      hostTime);
  }
  
  return rval;
}

bool AudioIO_IOS::audioProcessingCallbackCb(
   float **inputBuffers,
   unsigned int inputChannels,
   float **outputBuffers,
   unsigned int outputChannels,
   unsigned int numberOfFrames,
   unsigned int samplerate,
   unsigned long long hostTime){
  
// bool (*audioProcessingCallback) (void *clientdata, short int *audioIO, int numberOfFrames, int samplerate);
  
  if (inputBuffers){
    // Ignore input buffers
    (void)inputBuffers;
  }
  
  if (outputBuffers){
    const size_t numSamples = outputChannels * numberOfFrames;
    const int numBytes = (int)(numSamples * sizeof(int16_t));
    int16_t * pOutputBuf = (int16_t *)mAudioBuf.u8DataPtr(numBytes, numBytes);
    
    bool filledOk = false;
    if (mCallbackFn){
      filledOk = mCallbackFn(mpClientData, pOutputBuf, numberOfFrames, samplerate );
    }
    if (filledOk){
      const float div = 1.0f/32768.0f;
      for (unsigned int ch = 0; ch < outputChannels; ch++){
        float * const chOut = outputBuffers[ch];
        for (unsigned int frame = 0; frame < numberOfFrames; frame++){
          chOut[frame]= div*pOutputBuf[ch + frame*outputChannels];
        }
      }
    }
    else {
      memset(pOutputBuf, 0, mAudioBuf.length());
    }
  }
  
  return true;
}
  
AudioIO_IOS::~AudioIO_IOS(){
  LOG_TRACE(("Destructing AudioIO_IOS\r\n"));
}

void AudioIO_IOS::onForeground(){
}

void AudioIO_IOS::onBackground() {
}

void AudioIO_IOS::start() {
}

void AudioIO_IOS::stop() {
}
  

AudioIO *AudioIO::createNew(
   int samplerate,
   int buffersize,
   bool enableInput,
   bool enableOutput,
   audioProcessingCallback callback,
   void *clientdata,
   int inputStreamType,
   int outputStreamType
    )
{
  return new AudioIO_IOS(
    samplerate,
    buffersize,
    enableInput,
    enableOutput,
    callback,
    clientdata,
    inputStreamType,
    outputStreamType);
}
