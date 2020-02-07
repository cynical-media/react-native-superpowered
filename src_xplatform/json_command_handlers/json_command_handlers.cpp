/**
* COPYRIGHT    (c)  Applicaudia 2018
* @file        json_command_handlers.cpp
* @brief       Actions for json commands from RN, for now a template.
*/
#include "json_command_handlers.hpp"

#include "task_sched/task_sched.h"
#include "osal/platform_type.h"
#include "media_player/media_player.hpp"

#if (TARGET_OS_ANDROID)
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <OpenSource/SuperpoweredAndroidAudioIO.h>
#endif


#include "nlohmann/json.hpp"
#include "phone_al/json_commands.hpp"
#include "utils/platform_log.h"
#include "utils/convert_utils.h"
#include "utils/simple_string.hpp"
#include "utils/helper_macros.h"
#include <cstring>

LOG_MODNAME("json_command_handlers.cpp");

extern void JsonCommandInit(const CmdHandlerNode *pAry, const int len);

using namespace nlohmann;

static const char cmd_invalid[] = "cmd_invalid";
static const char cmd_initialize_superpowered[] = "cmd_initialize_superpowered";
//static const char cmd_gen_tone[] = "cmd_gen_tone";
//static const char cmd_start_recording[] = "cmd_start_recording";
//static const char cmd_stop_recording[] = "cmd_stop_recording";
static const char cmd_start_playback[] = "cmd_start_playback";
static const char cmd_stop_playback[] = "cmd_stop_playback";
static const char cmd_set_playback_speed[] = "cmd_set_playback_speed";
static const char cmd_set_playback_direction[] = "cmd_set_playback_direction";

class MPHolder {
public:

  static MPHolder &inst(){
    static MPHolder theInst;
    return theInst;
  }

  MPHolder()
  : mpMedia(new MediaPlayer())
  {

  }

  ~MPHolder(){
    delete mpMedia;
  }

  MediaPlayer *getMP(){
    return mpMedia;
  }

private:
  MediaPlayer *const mpMedia;
};

static bool OnUnhandledCmd(CmdHandlerNodeData * const pCmdData) {
  const json &jIn = pCmdData->jsonIn;
  json &jOut = pCmdData->jsonOut;
  (void)jIn;
  (void)jOut;
  //std::cerr << "Unhandled command!" << std::endl;
  return true;
}


typedef struct InitializeSuperpoweredCmdTag {
  std::string licenseKey;
  bool enableAudioAnalysis;
  bool enableFFTAndFrequencyDomain;
  bool enableAudioTimeStretching;
  bool enableAudioEffects;
  bool enableAudioPlayerAndDecoder;
  bool enableCryptographics;
  bool enableNetworking;
} InitializeSuperpoweredCmd;

void from_json(const nlohmann::json& j, InitializeSuperpoweredCmd& s) {
  s.licenseKey                    = j.value("licenseKey", "");
  s.enableAudioAnalysis           = j.value("enableAudioAnalysis", false);
  s.enableFFTAndFrequencyDomain   = j.value("enableFFTAndFrequencyDomain", false);
  s.enableAudioTimeStretching     = j.value("enableAudioTimeStretching", true);
  s.enableAudioEffects            = j.value("enableAudioEffects", false);
  s.enableAudioPlayerAndDecoder   = j.value("enableAudioPlayerAndDecoder", true);
  s.enableCryptographics          = j.value("enableCryptographics", false);
  s.enableNetworking              = j.value("enableNetworking", true);
}

static bool OnInitializeSuperpoweredCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  InitializeSuperpoweredCmd s = jIn;
  static std::string licenceKey = (s.licenseKey.length() <= 2) ? "ExampleLicenseKey-WillExpire-OnNextUpdate" : s.licenseKey;
  Superpowered::Initialize(
    licenceKey.c_str(),
    s.enableAudioAnalysis,
    s.enableFFTAndFrequencyDomain,
    s.enableAudioTimeStretching,
    s.enableAudioEffects,
    s.enableAudioPlayerAndDecoder,
    s.enableCryptographics,
    s.enableNetworking);
  json &jOut = pCmdData->jsonOut;
  (void)jOut;
  return true;
}

#if 0
typedef struct ToneCmdTag {
  float freq;
  float scale;
} ToneCmd;

void from_json(const nlohmann::json& j, ToneCmd& s) {
  s.freq = j.value("freq", 0.0f);
  s.scale = j.value("scale", 0.0f);
}

static bool OnToneCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  ToneCmd s = jIn;
  json &jOut = pCmdData->jsonOut;
  jOut["toneCmd"] = "ok";
  LOG_TRACE(("Got cmd_tone with freq = %f and scale = %f\r\n", s.freq, s.scale));
  return true;
}

typedef struct StartRecordingCmdTag {
  std::string filePath;
  float sampleRate;
  float seconds;
} StartRecordingCmd;

void from_json(const nlohmann::json& j, StartRecordingCmd& s) {
  s.filePath = j.value("filePath", "");
  s.sampleRate = j.value("sampleRate", 48000.0f);
  s.seconds = j.value("seconds", 10.0f);
}

static bool OnStartRecordingCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  StartRecordingCmd s = jIn;
  json &jOut = pCmdData->jsonOut;

  return true;
}

typedef struct StopRecordingCmdTag {
  std::string dummy;
} StopRecordingCmd;

void from_json(const nlohmann::json& j, StopRecordingCmd& s) {
  s.dummy = j.value("dummy", "");
}

static bool OnStopRecordingCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  StopRecordingCmd s = jIn;
  json &jOut = pCmdData->jsonOut;
  //jOut["toneCmd"] = "ok";
  //LOG_TRACE(("Got cmd_tone with freq = %f and scale = %f\r\n", s.freq, s.scale));
  return true;
}

#endif

typedef struct StartPlaybackCmdTag {
  std::string filePath;
} StartPlaybackCmd;

void from_json(const nlohmann::json& j, StartPlaybackCmd& s) {
  s.filePath = j.value("filePath", "");
}
static bool OnStartPlaybackCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  StartPlaybackCmd s = jIn;
  if (s.filePath.length() > 0 ) {
    LOG_TRACE(("s.filepath = %s", s.filePath.c_str()));
    MediaPlayer *player = MPHolder::inst().getMP();
    if (0 == s.filePath.compare(0, 4, "http")){      
      player->open(s.filePath.c_str());
    }
    else {
      player->open(s.filePath.c_str());
    }
  }
  json &jOut = pCmdData->jsonOut;
  (void)jOut;
  return true;
}

typedef struct StopPlaybackCmdTag {
  uint8_t dummy;
} StopPlaybackCmd;

void from_json(const nlohmann::json& j, StopPlaybackCmd& s) {
  s.dummy = 0;
}
static bool OnStopPlaybackCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  StopPlaybackCmd s = jIn;
  (void)s;
  MediaPlayer *player = MPHolder::inst().getMP();
  player->stop();
  json &jOut = pCmdData->jsonOut;
  (void)jOut;
  return true;
}

typedef struct PlaybackSpeedCmdTag {
  float speed;
} PlaybackSpeedCmdTag;


void from_json(const nlohmann::json& j, PlaybackSpeedCmdTag& s) {
  s.speed = j.value("speed", 1.0f);
}
static bool OnSetPlaybackSpeedCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  PlaybackSpeedCmdTag s = jIn;
  MediaPlayer *player = MPHolder::inst().getMP();
  player->setSpeed(s.speed);
  json &jOut = pCmdData->jsonOut;
  (void)jOut;
  return true;
}

typedef struct SetPlaybackDirectionCmdTag {
  bool reverse;
} SetPlaybackDirectionCmdTag;

void from_json(const nlohmann::json& j, SetPlaybackDirectionCmdTag& s) {
  s.reverse = j.value("reverse", false);
}
static bool OnSetPlaybackDirectionCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  SetPlaybackDirectionCmdTag s = jIn;
  MediaPlayer *player = MPHolder::inst().getMP();
  player->setDirection(s.reverse);
  json &jOut = pCmdData->jsonOut;
  (void)jOut;
  return true;
}

const CmdHandlerNode CmdHandlerNodeAry[] = {
    {cmd_invalid,                   OnUnhandledCmd},
    {cmd_initialize_superpowered,   OnInitializeSuperpoweredCmd},
#if 0
    {cmd_gen_tone,                  OnToneCmd},
    {cmd_start_recording,           OnStartRecordingCmd},
    {cmd_stop_recording,            OnStopRecordingCmd},
#endif
    {cmd_start_playback,            OnStartPlaybackCmd},
    {cmd_stop_playback,             OnStopPlaybackCmd},
    {cmd_set_playback_speed,        OnSetPlaybackSpeedCmd},
    {cmd_set_playback_direction,    OnSetPlaybackDirectionCmd},

};

const int CmdHandlerNodeAryLen = sizeof(CmdHandlerNodeAry) / sizeof(CmdHandlerNodeAry[0]);

void JsonRegisterCommands(void){
  JsonCommandInit(CmdHandlerNodeAry, CmdHandlerNodeAryLen);
}
