package com.x86kernel.rnsuperpowered;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Process;
import android.util.Log;

import com.facebook.react.bridge.Arguments;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.WritableMap;
import com.facebook.react.modules.core.DeviceEventManagerModule;

public class SuperpoweredJni {


    private static Object syncObj = new Object();
    private static HandlerThread mHandlerThread;
    private static Handler mHandler;
    static {
        System.loadLibrary("SuperpoweredJni");
    }

    private SuperpoweredJni(){
        mHandlerThread = new HandlerThread("HandlerThread",
            Process.THREAD_PRIORITY_MORE_FAVORABLE);
        if (null != mHandlerThread) {
            mHandlerThread.start();
            Looper looper = mHandlerThread.getLooper();
            if (null != looper) {
                mHandler = new Handler(looper);
            }
        }
    }

    private static SuperpoweredJni instance = null;

    public static SuperpoweredJni inst(){
        synchronized (syncObj){
            if (null == instance){
                instance = new SuperpoweredJni();
                SuperpoweredJni.init();
            }
        }
        return instance;
    }

    private static native void init();

    public native void nativeJsonCommand(String jsonCmd);

    private native void nativeExecuteRunnable(Object objectPointer);

    // Called by native functions
    public void jniTrace(String str){
        Log.d("CppBridge", str);
    }

    public void jniJsonResponse(String str){
        final ReactApplicationContext ctx = RNSuperpoweredModule.getReactContextSingleton();
        if (null != ctx) {
            final String fstring = new String(str);
            {
                WritableMap params = Arguments.createMap();
                params.putString("json", fstring);
                DeviceEventManagerModule.RCTDeviceEventEmitter js =
                    ctx.getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter.class);
                js.emit("RnJsonEvent", params);
            }
        }
    }

    // Called by the by C/C++ code via JNI!
    private class MyJniRunner implements Runnable{
        final Object mObjectPointer;

        MyJniRunner(Object objectPointer){
            mObjectPointer = objectPointer;
        }

        @Override
        public void run() {
            try {
                nativeExecuteRunnable(mObjectPointer);
                this.finalize();
            }
            catch(Exception e){
                Log.e("Crash!!", "Crash on NExecuteRunnable!", e);
            } catch (Throwable throwable) {
                throwable.printStackTrace();
            }
        }
    }

    public Object jniRunOnUiThread( Object objectPointer, int delayMs ) {
        final MyJniRunner runner = new MyJniRunner(objectPointer);
        final ReactApplicationContext ctx = RNSuperpoweredModule.getReactContextSingleton();
        if (delayMs <= 0) {
            ctx.runOnNativeModulesQueueThread(runner);
        } else {
            Runnable delayedRunner = new Runnable() {
                @Override
                public void run() {
                    ctx.runOnNativeModulesQueueThread(runner);
                }
            };
            mHandler.postDelayed(delayedRunner, delayMs);
        }
        return runner;
    }

}
