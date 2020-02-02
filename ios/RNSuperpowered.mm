
#import "RNSuperpowered.h"
#import "Recorder.h"
#import "Audio.h"
#import "phone_al/phone_al.hpp"
#import "phone_al/json_commands.h"
#import "phone_al/json_commands_impl.hpp"

static RNSuperpowered * pReactSvcInst = nullptr;

@implementation RNSuperpowered

- (dispatch_queue_t)methodQueue
{
    pReactSvcInst = self;
    return dispatch_get_main_queue();
}
RCT_EXPORT_MODULE();

RCT_EXPORT_METHOD(startRecord:(NSInteger)sampleRate minSeconds:(NSInteger)minSeconds numChannels:(NSInteger)numChannels applyFade:(BOOL)applyFade) {
    Recorder *recorder = [Recorder createInstance: sampleRate minSeconds:minSeconds numChannels:numChannels applyFade:applyFade];
    
    [recorder startRecord: @"audio"];
}

RCT_REMAP_METHOD(stopRecord,
                 resolver:(RCTPromiseResolveBlock)resolve
                 rejecter:(RCTPromiseRejectBlock)reject) {
    Recorder *recorder = [Recorder getInstance];
    NSString *destPath = [recorder stopRecord];
    
    resolve(destPath);
}

RCT_EXPORT_METHOD(initializeAudio:(NSString *)filePath sampleRate:(NSInteger)sampleRate) {
    Audio *audio = [Audio createInstance:sampleRate];
    [audio loadFile:filePath];
}

RCT_EXPORT_METHOD(loadFile:(NSString *)filePath) {
    [[Audio getInstance] loadFile:filePath];
}

RCT_EXPORT_METHOD(playAudio) {
    [[Audio getInstance] play];
}

RCT_EXPORT_METHOD(pauseAudio) {
    [[Audio getInstance] pause];
}

RCT_EXPORT_METHOD(setEcho:(float)mix) {
    [[Audio getInstance] setEcho:mix];
}

RCT_EXPORT_METHOD(setPitchShift:(int)pitchShift) {
    [[Audio getInstance] setPitchShift:pitchShift];
}

RCT_REMAP_METHOD(process,
                 filePath:(NSString *)fileName
                 resolver:(RCTPromiseResolveBlock)resolve
                 rejecter:(RCTPromiseRejectBlock)reject) {
    Audio *audio = [Audio getInstance];
    
    @try {
        NSString *filePath = [audio process:fileName];
        
        NSMutableDictionary *response = [[NSMutableDictionary alloc] init];
        response[@"uri"] = filePath;
        response[@"isSuccess"] = @YES;
            
        resolve(response);
        
    } @catch (NSException *exception) {
        reject(exception.name, exception.reason, nil);
    }
}

// /////////////////////////////////////////////////////////////////////////////
// Return an array of the events softed by this object.
- (NSArray<NSString *> *)supportedEvents
{
  return @[@"RnJsonEvent"];
}

// /////////////////////////////////////////////////////////////////////////////
// Send JSON to react native
- (void)sendJsonEvent:(NSString *)jsonStr
{
  [self sendEventWithName:@"RnJsonEvent" body:@{@"json": jsonStr}];
}

// /////////////////////////////////////////////////////////////////////////////
// JsonHandler is the bridge between the RN code and the objective C code.
// It sends new JSON commands to the BleCommand layer of the PAKP state machine.
class JsonHandler {
public:
  static JsonHandler &inst(){
    static JsonHandler me;
    return me;
  }
  
  // Create and SEND a JSON command to the PAKP state machine.
  void jsonCommand(const char * const pJsonCommandStr){
    JsonCommand *pcmd = new JsonCommand(pJsonCommandStr, OnJsonCompletedCb, this, false);
    (void)pcmd; // the command registers itself when it is constructed.
  }
  
private:
  // Callback called on asynchronous json events.
  static void HandleJsonEvent(
    void * pUserData,
    const char * const pJson,
    int len)
  {
    NSString *nsJson = [NSString stringWithUTF8String:pJson];
    [pReactSvcInst sendJsonEvent:nsJson];
  }
  
  // Callback called by PAKP state machine when a JSON command is completed.
  static void OnJsonCompletedCb (
   void * const pUserData,
   const bool status,
   const char * const pJsonEvent)
  {
    (void)pUserData;
    NSString *nsJson = [NSString stringWithUTF8String:pJsonEvent];
    if (nsJson){
      [pReactSvcInst sendJsonEvent:nsJson];
    }
  }
  
  // Constructor initializes BleApple.mm
  JsonHandler(){
    //BleApple_Init(HandleJsonEvent, NULL);
  }
};

// ///////////////////////////////////////////////////////////////////////
RCT_EXPORT_METHOD(jsonCommand:(NSString *)nsCmd)
{
  const char *jsonCmd = [nsCmd cStringUsingEncoding: NSUTF8StringEncoding];
  JsonHandler::inst().jsonCommand(jsonCmd);
}

@end
