#include "jni_locker.hpp"
#include "jni_utils.hpp"
#include "Audio.h"
#include "json_commands/json_commands.hpp"
#include "osal/cs_task_locker.hpp"
#include "phone_al/phone_al.hpp"
#include "phone_al/json_handler.hpp"
#include "utils/platform_log.h"
#include "task_sched/task_sched.h"
#include "osal/osal.h"

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


static void android_LogFn(void *pUserData, const uint32_t ts, const char *szLine, const int len) {
#ifdef ANDROID
#if FALSE
  if (pEnv) {
    bn_fetchMethodIdIfNotDefined(&ble.d.midTrace, "jniTrace",
                                 "(Ljava/lang/String;)V");

    jstring string = pEnv->NewStringUTF(szLine);
    pEnv->CallVoidMethod(pObj, ble.d.midTrace, string);
    pEnv->DeleteLocalRef(string);

  } else
#endif
  {
    __android_log_write(ANDROID_LOG_DEBUG, "JniNative:", szLine);
  }
#endif
}

static JniLockerClass jnilocker;

class GlobalsLocker {
public:
  GlobalsLocker(JNIEnv *pEnv, jobject jObj) {
    CSTaskLocker locker;
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


static void bn_fetchMethodIdIfNotDefined(jmethodID *pMeth, const char *name,
                                         const char *sig) {
  _JNIEnv * const pEnv = jnilocker.getEnvPtr();
  _jclass * const pCls = jnilocker.getClsPtr();
  if (*pMeth == nullptr) {
    *pMeth = pEnv->GetMethodID(pCls, name, sig);
  }
}


typedef struct BnRunnableTag {
  PhoneAL::PAKAL_RunnableCb cb;
  void *pPakObj;
  int delayMs;
  jobject    jGlobalRef;

  BnRunnableTag(
      PhoneAL::PAKAL_RunnableCb cb,
      void *pPakObj = nullptr,
      int delayMs = 0)
      : cb(cb)
      , pPakObj(pPakObj)
      , delayMs(delayMs)
      , jGlobalRef(nullptr)
  {
  }
} BnRunnableT;

extern "C" {
extern void JsonRegisterCommands(void);
}

class HalClass: PhoneAL {
public:
  jmethodID midTrace;
  jmethodID midJsonResponse;

  // jclass  jcls;
  jmethodID midRunOnUiThread;
  int runs;

public:
  // //////////////////////////////////////////////////////////////////////////////////
  HalClass()
  : midTrace(0)
  , midJsonResponse(0)
  , midRunOnUiThread(0)
  , runs(0)
  {
    OSALInit();
    TaskSchedInit();
    LOG_Init(android_LogFn, nullptr);
    JsonRegisterCommands();
    PakSchedInit(this);
  }

  // //////////////////////////////////////////////////////////////////////////////////
  ~HalClass(){
    CSTaskLocker cs;
    PakSchedInit(nullptr);
  }

  // //////////////////////////////////////////////////////////////////////////////////
  // Executes a runnable from the "UI" thread.  Passes back a pointer that can be
  // used to cancel it.
  void *RunOnUiThread(
      PhoneAL::PAKAL_RunnableCb cb,
      void *const pObj,
      const int delayMs) override {
    void *rval = NULL;
    jboolean jFalse = false;
    //([BI)Ljava/lang/Object;
    bn_fetchMethodIdIfNotDefined(&midRunOnUiThread, "jniRunOnUiThread",
                                 "(Ljava/lang/Object;I)Ljava/lang/Object;");
    LOG_ASSERT(midRunOnUiThread);
    if (midRunOnUiThread) {
      runs++;

      BnRunnableT * const pRunnable = new BnRunnableT(cb, pObj, delayMs);//(BnRunnableT *)pBytePtr;
      LOG_ASSERT(pRunnable);
      if (pRunnable) {

        _JNIEnv * const pEnv = jnilocker.getEnvPtr();
        _jobject * const pObj = jnilocker.getObjPtr();
        _jclass * const pCls = jnilocker.getClsPtr();
        jobject jDirectByteBuffer = pEnv->NewDirectByteBuffer(pRunnable, sizeof(BnRunnableT));
        pRunnable->jGlobalRef = pEnv->NewGlobalRef(jDirectByteBuffer);
        pEnv->CallObjectMethod(pObj, midRunOnUiThread,
                               jDirectByteBuffer, (jint) delayMs);
      }

      rval = (void *)pRunnable;
    } else {
      LOG_ASSERT(0);
    }
    return rval;
  }

// //////////////////////////////////////////////////////////////////////////////////
// Cancels a runnable.  Uses the pointer passed back from RunOnUiThread.
  void CancelRunnable(void *pRunnableObj) override {
    LOG_ASSERT(pRunnableObj);
    BnRunnableT *pRunnable = (BnRunnableT *)pRunnableObj;
    LOG_VERBOSE(("bn_CancelRunnable(0x%x)", pRunnable));
    if (pRunnable) {
      pRunnable->cb = NULL;
    }

    // Let the callback be called - only then is it safe to delete it.
  }

};

static HalClass pakhal;

static JsonHandler jsonHandler;
#include "phone_al/json_commands.h"

// ////////////////////////////////////////////////////////////////////////////
JsonHandler &JsonHandler::inst(){
  return jsonHandler;
}

// ////////////////////////////////////////////////////////////////////////////
void JsonHandler::OnJsonCompletedCb(
    void * const pUserData, const bool status, const char * const pJsonEvent) {

  _JNIEnv * const pEnv = jnilocker.getEnvPtr();
  _jobject * const pObj = jnilocker.getObjPtr();

  if (pEnv) {
    bn_fetchMethodIdIfNotDefined(&pakhal.midJsonResponse, "jniJsonResponse",
                                 "(Ljava/lang/String;)V");

    LOG_ASSERT(pakhal.midJsonResponse);

    jstring string = pEnv->NewStringUTF(pJsonEvent);
    pEnv->CallVoidMethod(pObj, pakhal.midJsonResponse, string);
    pEnv->DeleteLocalRef(string);
  }
}



// ////////////////////////////////////////////////////////////////////////////
void JsonHandler::jsonCommand(const char * const jsonCommand){
  JsonCommand *pcmd = new JsonCommand(jsonCommand, OnJsonCompletedCb, this);
  (void)pcmd;
}

// ////////////////////////////////////////////////////////////////////////////
void JsonHandler::JsonEvent(void *const pConfigUserData,
                            const char * const pJson,
                            const int strLen) {

  (void)pConfigUserData;
  _JNIEnv * const pEnv = jnilocker.getEnvPtr();
  _jobject * const pObj = jnilocker.getObjPtr();

  if (pEnv) {
    bn_fetchMethodIdIfNotDefined(
        &pakhal.midJsonResponse,
        "jniJsonResponse",
        "(Ljava/lang/String;)V");

    LOG_ASSERT(pakhal.midJsonResponse);

    jstring string = pEnv->NewStringUTF(pJson);
    pEnv->CallVoidMethod(pObj, pakhal.midJsonResponse, string);
    pEnv->DeleteLocalRef(string);
  }
}

// ////////////////////////////////////////////////////////////////////////////
JsonHandler::JsonHandler() {
  LOG_ASSERT(this == &jsonHandler);
}

// ////////////////////////////////////////////////////////////////////////////
// END JsonHandler
// ////////////////////////////////////////////////////////////////////////////



extern "C"
JNIEXPORT void Java_com_x86kernel_rnsuperpowered_SuperpoweredJni_init(
		JNIEnv *env,
    jclass obj
) {
  GlobalsLocker lock(env, (jobject)obj);
  (void)lock;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_x86kernel_rnsuperpowered_SuperpoweredJni_jsonCommand(
    JNIEnv *pEnv,
    jobject thiz,
    jstring jsonCmd)
{
  GlobalsLocker jniGlobals(pEnv, thiz);
  (void)jniGlobals;
  std::string sJson;
  {
    JStringGetter cJson(pEnv, jsonCmd);
    sJson = cJson.getString();
  }



}
