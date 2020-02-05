#import "audio_io.hpp"
#import "audio_io_ios.hpp"
#import "SuperpoweredIOSAudioIO.h"
#include "Superpowered.h"
#include "SuperpoweredAdvancedAudioPlayer.h"
#include "SuperpoweredSimple.h"

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
  
  void *mpSuperpowered;
  NSObject *mpDelegateSelf;
  AudioIO_IOS_objc *mpAudio;

};

@implementation AudioIO_IOS_objc {
  SuperpoweredIOSAudioIO *_audioIO;
  AudioIO_IOS *_audioIOClass;
  void *_userClientData;
    //Superpowered::AdvancedAudioPlayer *player;
}

- (id)initWithDelegate:(NSObject *)d
            withClass : (AudioIO_IOS *)pClass
            preferredBufferSize:(unsigned int)prefBufferSize
            preferredSamplerate:(unsigned int)prefsamplerate
            channels:(int)channels
            audioProcessingCallback:(audioProcessingCallback)callback
            clientdata:(void *)clientdata
{
  self = [super init];
  if (self) {
    _audioIO = [[SuperpoweredIOSAudioIO alloc]
    initWithDelegate:(id<SuperpoweredIOSAudioIODelegate>)self
    preferredBufferSize:12
    preferredSamplerate:44100
    audioSessionCategory:AVAudioSessionCategoryPlayback
    channels:2
    audioProcessingCallback:callback
    clientdata:(__bridge void *)self];
    
    _audioIOClass = pClass;
    _userClientData = clientdata;

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
    int outputStreamType){
  
  mpDelegateSelf = [[NSObject alloc] init]; // Just used as a selector
  
  // Pointer to the objective C interface.
  mpAudio = [[AudioIO_IOS_objc alloc]
                              initWithDelegate:mpDelegateSelf
                              withClass: this
                              preferredBufferSize:buffersize
                              preferredSamplerate:samplerate
                              channels:2
                              audioProcessingCallback:audioProcessingCallbackCbC
                              clientdata:clientdata];
    
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
  
  AudioIO_IOS_objc *pObjC = (__bridge AudioIO_IOS_objc *)clientData;
  AudioIO_IOS *pThis = [pObjC getClass];
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
  return true;
}
  
AudioIO_IOS::~AudioIO_IOS(){
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
  return new AudioIO_IOS(samplerate, buffersize, enableInput, enableOutput, callback, clientdata, inputStreamType, outputStreamType);
}
