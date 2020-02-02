#include "jni_locker.hpp"
#include "jni_utils.hpp"
#include "Audio.h"
#include "json_commands/json_commands.hpp"
#include "osal/cs_task_locker.hpp"
#include "phone_al/phone_al.hpp"
#include "utils/platform_log.h"

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

typedef void (*PAKAL_RunnableCb)(void *pObj);

typedef struct BnRunnableTag {
  PAKAL_RunnableCb cb;
  void *pPakObj;
  int delayMs;
  jobject    jGlobalRef;

  BnRunnableTag(
      PAKAL_RunnableCb cb,
      void *pPakObj = nullptr,
      int delayMs = 0)
      : cb(cb)
      , pPakObj(pPakObj)
      , delayMs(delayMs)
      , jGlobalRef(nullptr)
  {
  }
} BnRunnableT;

class HalClass: PhoneAL {
public:
  // //////////////////////////////////////////////////////////////////////////////////
  HalClass(){
    //PakSchedInit(this);
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
    bn_fetchMethodIdIfNotDefined(&ble.d.midRunOnUiThread, "jniRunOnUiThread",
                                 "(Ljava/lang/Object;I)Ljava/lang/Object;");
    LOG_ASSERT(ble.d.midRunOnUiThread);
    if (ble.d.midRunOnUiThread) {
      runs++;

      BnRunnableT * const pRunnable = new BnRunnableT(cb, pObj, delayMs);//(BnRunnableT *)pBytePtr;
      LOG_ASSERT(pRunnable);
      if (pRunnable) {

        _JNIEnv * const pEnv = locker.getEnvPtr();
        _jobject * const pObj = locker.getObjPtr();
        _jclass * const pCls = locker.getClsPtr();
        jobject jDirectByteBuffer = pEnv->NewDirectByteBuffer(pRunnable, sizeof(BnRunnableT));
        pRunnable->jGlobalRef = pEnv->NewGlobalRef(jDirectByteBuffer);
        pEnv->CallObjectMethod(pObj, ble.d.midRunOnUiThread,
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

// //////////////////////////////////////////////////////////////////////////////////
  void TryMicrophone(void * pConfigUserData, const bool start) {
    LOG_VERBOSE(("TryMicrophone()"));

    // public void jniTrySubscribe(int, java.lang.String);
    // descriptor: (Z)V
    bn_fetchMethodIdIfNotDefined(&ble.d.midTryMicrophone, "jniTryMicrophone",
                                 "(Z)V");

    if (ble.d.midTryMicrophone) {
      LOG_ASSERT(ble.d.midTryMicrophone);

      jboolean jstart = (start) ? 1 : 0;
      _JNIEnv * const pEnv = locker.getEnvPtr();
      _jobject * const pObj = locker.getObjPtr();

      pEnv->CallVoidMethod(pObj, ble.d.midTryMicrophone, jstart);
    }
  }

};

static HalClass pakhal;


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

  jsonCommand(sJson);

}
