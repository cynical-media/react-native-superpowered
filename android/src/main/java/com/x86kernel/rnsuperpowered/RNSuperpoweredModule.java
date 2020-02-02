package com.x86kernel.rnsuperpowered;

import java.util.HashMap;

import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.bridge.Callback;
import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.WritableMap;
import com.facebook.react.bridge.WritableNativeMap;

public class RNSuperpoweredModule extends ReactContextBaseJavaModule {
  private static ReactApplicationContext _reactContext;

  public RNSuperpoweredModule(ReactApplicationContext reactContext) {
    super(reactContext);
    _reactContext = reactContext;
  }

  public static ReactApplicationContext getReactContextSingleton() {
	  return _reactContext;
  }

  @Override
  public String getName() {
    return "RNSuperpowered";
  }

  @ReactMethod
  public void startRecord(int sampleRate, int minSeconds, int numChannels, boolean applyFade) {
    Recorder recorder = Recorder.createInstance("temp.wav", sampleRate, minSeconds, numChannels, applyFade);

    if(recorder != null) {
        recorder.start("audio");
    }
  }

  @ReactMethod
  public void stopRecord(Promise promise) {
      promise.resolve(Recorder.getInstance().stop());
  }

  @ReactMethod
  public void initializeAudio(String filePath, int sampleRate) {
      Audio audio = Audio.createInstance(sampleRate);
	  audio.loadFile(filePath);
  }

  @ReactMethod
  public void loadFile(String filePath) {
      Audio.getInstance().loadFile(filePath);
  }

  @ReactMethod
  public void playAudio() {
      Audio.getInstance().play();
  }

  @ReactMethod
  public void pauseAudio() {
      Audio.getInstance().pause();
  }

  @ReactMethod
  public void stopAudio() {
	  Audio audio = Audio.getInstance();

	  audio.pause();
      audio.setPosition(0);
  }

  @ReactMethod
  public void setPosition(double ms) {
      Audio.getInstance().setPosition(ms);
  }

  @ReactMethod
  public void setEcho(float mix) {
      Audio.getInstance().setEcho(mix);
  }

  @ReactMethod
  public void setPitchShift(int pitchShift) {
      Audio.getInstance().setPitchShift(pitchShift);
  }

  @ReactMethod
  public void process(String filePath, Promise promise) {
      String outputFile = Audio.getInstance().processToFile(filePath);
    
      WritableMap response = new WritableNativeMap();
      response.putString("uri", outputFile);
      response.putBoolean("isSuccess", true);
      
      promise.resolve(response);
  }

  @ReactMethod
  public void jsonCommand(String jsonCmd){
    SuperpoweredJni.inst().nativeJsonCommand(jsonCmd);
  }
}
