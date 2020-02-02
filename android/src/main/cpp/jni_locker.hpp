#ifndef JNI_LOCKER_HPP
#define JNI_LOCKER_HPP

#include <jni.h>

class JniLockerClass {
private:
  // memsettable.
  _JNIEnv *mpEnv;
  _jobject *mjobj;
  _jclass *mjcls;
  _jobject *mjclsref;
  int nesting;

public:

  JniLockerClass()
      : mpEnv(nullptr)
      , mjobj(nullptr)
      , mjcls(nullptr)
      , mjclsref(nullptr)
      , nesting(0)
  {
  }

  void doEnterJni(_JNIEnv *pEnv, _jobject *jObj) {
    if (++nesting == 1) {
      mpEnv = pEnv;
      mjobj = jObj;
      mjcls = (jObj) ? pEnv->GetObjectClass(mjobj) : nullptr;
      mjclsref = (mjcls) ? pEnv->NewGlobalRef(mjcls) : nullptr;
    }
  }

  void doExitJni() {
    if (--nesting == 0) {
      if (mjobj) {
        if (mjclsref) {
          mpEnv->DeleteGlobalRef(mjclsref);
        }
        mpEnv = nullptr;
        mjcls = nullptr;
        mjclsref = nullptr;
      }
    }
  }

  _JNIEnv *getEnvPtr(){
    return mpEnv;
  }

  _jobject *getObjPtr(){
    return mjobj;
  }

  _jclass *getClsPtr(){
    return mjcls;
  }

};



#endif