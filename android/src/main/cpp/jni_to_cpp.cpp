#include "jni_locker.hpp"
#include "jni_utils.hpp"
#include "Audio.h"
#include <SuperpoweredSimple.h>
#include <SuperpoweredCPU.h>
#include <SuperpoweredDecoder.h>
#include <SuperpoweredRecorder.h>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <thread>
#include <mutex>

#define log_print __android_log_print

static JniLockerClass jnilocker;

class GlobalsLocker {
public:
  GlobalsLocker(JNIEnv *pEnv, jobject jObj) {
    std::lock_guard<std::mutex> lock(myMutex);
    jnilocker.doEnterJni(pEnv, jObj);
  }
  ~GlobalsLocker() {
    std::lock_guard<std::mutex> lock(myMutex);
    jnilocker.doExitJni();
  }
private:
  static std::mutex myMutex;

};

// Used to ensure only one entry at a time.
std::mutex GlobalsLocker::myMutex;

extern "C"
JNIEXPORT void Java_com_x86kernel_rnsuperpowered_SuperpoweredJni_init(
		JNIEnv *env,
    jclass obj
) {
  GlobalsLocker lock(env, (jobject)obj);
  (void)lock;
}

// ////////////////////////////////////////////////////////////////////////////
extern "C"
JNIEXPORT void Java_com_reactlibrary_RnToCppModule_JSONCommand(
    JNIEnv *pEnv,
    jobject instance,
    jstring json) {
  GlobalsLocker jniGlobals(pEnv, instance);
  (void)jniGlobals;
  std::string sJson;
  {
    JStringGetter cJson(pEnv, json);
    sJson = cJson.getString();
  }
  //JsonHandler::inst().jsonCommand(sJson.c_str());
}
