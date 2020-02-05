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


#if (TARGET_OS_ANDROID > 0)
#ifndef __try
#define __try if(true)
#endif
#ifndef __catch
#define __catch(e) if(false)
#endif
#endif

#ifndef __try
#define __try try
#endif
#ifndef __catch
#define __catch(e) catch(e)
#endif

LOG_MODNAME("json_command_handlers.cpp");

extern void JsonCommandInit(const CmdHandlerNode *pAry, const int len);

using namespace nlohmann;

static const char cmd_invalid[] = "cmd_invalid";
static const char cmd_gen_tone[] = "cmd_gen_tone";
static const char cmd_start_recording[] = "cmd_start_recording";
static const char cmd_stop_recording[] = "cmd_stop_recording";
static const char cmd_start_playback[] = "cmd_start_playback";
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

typedef struct ToneCmdTag {
  float freq;
  float scale;
} ToneCmd;

void from_json(const nlohmann::json& j, ToneCmd& s) {
  s.freq = j.value("freq", 0.0f);
  s.scale = j.value("scale", 0.0f);
}

static bool onToneCmd(CmdHandlerNodeData * const pCmdData){
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

static bool onStartRecordingCmd(CmdHandlerNodeData * const pCmdData){
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

static bool onStopRecordingCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  StopRecordingCmd s = jIn;
  json &jOut = pCmdData->jsonOut;
  //jOut["toneCmd"] = "ok";
  //LOG_TRACE(("Got cmd_tone with freq = %f and scale = %f\r\n", s.freq, s.scale));
  return true;
}

typedef struct StartPlaybackCmdTag {
  std::string filePath;
} StartPlaybackCmd;

void from_json(const nlohmann::json& j, StartPlaybackCmd& s) {
  s.filePath = j.value("filePath", "");
}
static bool onStartPlaybackCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  StartPlaybackCmd s = jIn;
  if (s.filePath.length() > 0 ) {
    LOG_TRACE(("s.filepath = %s", s.filePath.c_str()));
    MediaPlayer *player = MPHolder::inst().getMP();
    player->open(s.filePath.c_str());
  }
  json &jOut = pCmdData->jsonOut;
  (void)jOut;
  return true;
}

typedef struct StartPlaybackSpeedCmdTag {
  float speed;
} StartPlaybackSpeedCmdTag;

void from_json(const nlohmann::json& j, StartPlaybackSpeedCmdTag& s) {
  s.speed = j.value("speed", 1.0f);
}
static bool onSetPlaybackSpeedCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  StartPlaybackSpeedCmdTag s = jIn;
  MediaPlayer *player = MPHolder::inst().getMP();
  player->setSpeed(s.speed);
  json &jOut = pCmdData->jsonOut;
  (void)jOut;
  return true;
}

typedef struct StartPlaybackDirectionCmdTag {
  bool reverse;
} StartPlaybackDirectionCmdTag;

void from_json(const nlohmann::json& j, StartPlaybackDirectionCmdTag& s) {
  s.reverse = j.value("reverse", false);
}
static bool onSetPlaybackDirectionCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  StartPlaybackDirectionCmdTag s = jIn;
  MediaPlayer *player = MPHolder::inst().getMP();
  player->setDirection(s.reverse);
  json &jOut = pCmdData->jsonOut;
  (void)jOut;
  return true;
}

const CmdHandlerNode CmdHandlerNodeAry[] = {
    {cmd_invalid,             OnUnhandledCmd},
    {cmd_gen_tone,            onToneCmd},
    {cmd_start_recording,     onStartRecordingCmd},
    {cmd_stop_recording,      onStopRecordingCmd},
    {cmd_start_playback,      onStartPlaybackCmd},
    {cmd_set_playback_speed,  onSetPlaybackSpeedCmd},
    {cmd_set_playback_direction,  onSetPlaybackDirectionCmd},

};

const int CmdHandlerNodeAryLen = sizeof(CmdHandlerNodeAry) / sizeof(CmdHandlerNodeAry[0]);

void JsonRegisterCommands(void){
  JsonCommandInit(CmdHandlerNodeAry, CmdHandlerNodeAryLen);
}
