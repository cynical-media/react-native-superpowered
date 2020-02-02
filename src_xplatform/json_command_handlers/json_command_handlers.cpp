#include "json_command_handlers.hpp"

/**
 * COPYRIGHT    (c)	Polestar 2018
 * @file        pak_command_handlers.cpp
 * @brief       Actions for json commands from RN.
 */
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

LOG_MODNAME("json_commands_crypto.cpp");

using namespace nlohmann;

static const char cmd_invalid[] = "cmd_invalid";
static const char cmd_tone[] = "cmd_tone";

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
  jOut["toneCmd"] = "tone_cmd";
  LOG_TRACE(("Got cmd_tone with freq = %f and scale = %f\r\n", s.freq, s.scale));
  return true;
}


extern void JsonCommandInit(const CmdHandlerNode *pAry, const int len);

const CmdHandlerNode CmdHandlerNodeAry[] = {
    {cmd_invalid,        OnUnhandledCmd},
    {cmd_tone,          onToneCmd},
};

const int CmdHandlerNodeAryLen = sizeof(CmdHandlerNodeAry) / sizeof(CmdHandlerNodeAry[0]);


extern "C" {

void JsonRegisterCommands(void){
  JsonCommandInit(CmdHandlerNodeAry, CmdHandlerNodeAryLen);
}

}
