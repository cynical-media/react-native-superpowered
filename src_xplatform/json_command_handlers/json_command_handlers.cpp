/**
* COPYRIGHT    (c)  Applicaudia 2018
* @file        json_command_handlers.cpp
* @brief       Actions for json commands from RN.
*/
#include "json_command_handlers.hpp"

#include <SuperpoweredSimple.h>
#include <SuperpoweredCPU.h>
#include <SuperpoweredDecoder.h>
#include <SuperpoweredRecorder.h>
#include "task_sched/task_sched.h"

#include "osal/platform_type.h"

#if (TARGET_OS_ANDROID)
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <AndroidIO/SuperpoweredAndroidAudioIO.h>
#include <SuperpoweredAdvancedAudioPlayer.h>
#include "Recorder.h"
#endif


#include "nlohmann/json.hpp"
#include "phone_al/json_commands_impl.hpp"
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

static Recorder *recorder = NULL;

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

  if (nullptr == recorder){
    recorder = new Recorder(
        s.filePath.c_str(),
        (int)(0.5+s.sampleRate/100.0),
        s.sampleRate, s.seconds, 2, false);
  }

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

static void SuperpoweredDecoderID3CallbackCbC(
    void *clientData,
    void *frameName,
    void *frameData,
    int frameDataSize){

}

static void SuperpoweredDecoderFullyDownloadedCallbackCbC(
    void *clientData,
    SuperpoweredDecoder *decoder){

  LOG_TRACE(("mp3 fully downloaded\r\n"));
}

struct decodeInfo{
  SuperpoweredDecoder *decoder;
  std::string filepath;

  decodeInfo(
      SuperpoweredDecoder *decoder
      , const char *filepath)
      : decoder(decoder)
      , filepath(filepath)
  {

  }

};

static void try_to_open(void *p, uint32_t){
  struct decodeInfo *pDecodeInf = (struct decodeInfo *)p;
  int statCode = 0;
  auto &decoder = *pDecodeInf->decoder;
  const char *errCode = decoder.open(
      pDecodeInf->filepath.c_str(),
      false,
      0,
      0,
      0,
      0,
      &statCode);
  if (nullptr == errCode){
    const unsigned int numberOfSamples = decoder.samplesPerFrame;
    short int *tmpBuf1 = (short int *) malloc(numberOfSamples * 4 + 16384);
    unsigned int nSamples1 = 0;
    unsigned char res = decoder.decode(tmpBuf1, &nSamples1);
    switch (res) {
      case SUPERPOWEREDDECODER_EOF:
        printf("Finished decoding user recording: %d\n", nSamples1);
        break;
      case SUPERPOWEREDDECODER_ERROR:
        printf("Error decoding user recording\n");
        break;
      case SUPERPOWEREDDECODER_OK:
      default:
        break;
    }

    free(tmpBuf1);

    delete pDecodeInf;
  }
  else {
    LOG_TRACE(("Got an error: %s\r\n", errCode));
    if (statCode == StatusCode_Progress) {
      TaskSchedScheduleFn(TS_PRIO_APP, try_to_open, p, 500);
    }
    else {
      delete pDecodeInf;
    }
  }
}

static bool onStartPlaybackCmd(CmdHandlerNodeData * const pCmdData){
  const json &jIn = pCmdData->jsonIn;
  StartPlaybackCmd s = jIn;
  if (s.filePath.length() > 0 ) {
    SuperpoweredAdvancedAudioPlayer::setTempFolder("/data/user/0/com.superpowered_test/cache");
    SuperpoweredDecoder *decoder = new SuperpoweredDecoder(
        SuperpoweredDecoderFullyDownloadedCallbackCbC,
        nullptr);

    struct decodeInfo *pDecodeInf = new decodeInfo(decoder, s.filePath.c_str());
    try_to_open(pDecodeInf, 0);

  }
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

};

const int CmdHandlerNodeAryLen = sizeof(CmdHandlerNodeAry) / sizeof(CmdHandlerNodeAry[0]);


extern "C" {

void JsonRegisterCommands(void){
  JsonCommandInit(CmdHandlerNodeAry, CmdHandlerNodeAryLen);
}

}
