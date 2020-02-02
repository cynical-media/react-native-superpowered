#ifndef JNI_LOCKER_HPP
#define JNI_LOCKER_HPP

#include <jni.h>

class JniLockerClass {
private:
  // memsettable.
  _JNIEnv *pEnv;
  _jobject *jobj;
  _jclass *jcls;
  _jobject *jclsref;
  int nesting;

public:

  JniLockerClass()
      : pEnv(nullptr)
      , jobj(nullptr)
      , jcls(nullptr)
      , jclsref(nullptr)
      , nesting(0)
  {
  }

  void doEnterJni(_JNIEnv *pEnv, _jobject *jObj) {
    if (++nesting == 1) {
      pEnv = pEnv;
      jobj = jObj;
      if (jObj){
        jcls = pEnv->GetObjectClass(jobj);
        jclsref = pEnv->NewGlobalRef(jcls);
      }
    }
  }

  void doExitJni() {
    if (--nesting == 0) {
      if (jobj) {
        pEnv->DeleteGlobalRef(jclsref);
        pEnv = NULL;
      }
    }
  }

  _JNIEnv *getEnvPtr(){
    return pEnv;
  }

  _jobject *getObjPtr(){
    return jobj;
  }

  _jclass *getClsPtr(){
    return jcls;
  }

};



#endif