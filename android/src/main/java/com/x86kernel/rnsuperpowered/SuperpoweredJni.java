package com.x86kernel.rnsuperpowered;

import android.os.Handler;
import android.os.HandlerThread;
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


    private static SuperpoweredJni instance = null;


    public static SuperpoweredJni inst(){
        synchronized (syncObj){
            if (null == instance){
                instance = new SuperpoweredJni();
            }
        }
        return instance;
    }

    private static native void init();

    public native void jsonCommand(String jsonCmd);

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

}
