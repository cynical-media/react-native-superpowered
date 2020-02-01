import { 
  NativeModules,
  PermissionsAndroid,
  Platform,
} from 'react-native'

const { RNSuperpowered } = NativeModules

var recorderInstance = null;

export default class Recorder {
  
  constructor() {
    console.log("Initializing new recorder instance.");
    this.init();
  }

  init = () => {
    if(Platform.OS === 'android') {
      return new Promise((resolve, reject) => {
        PermissionsAndroid.request(PermissionsAndroid.PERMISSIONS.RECORD_AUDIO)
          .then(result => {
            if(result === PermissionsAndroid.RESULTS.GRANTED || result === true)
              resolve(true);
            else
              resolve(false);
          })
      })
    }   
  }

  // --------------------------------------------------------------------------------------------
  static inst(){
    if (!recorderInstance) {
      console.log("Creating new recorder instance.");
      recorderInstance = new Recorder();
    }
    return recorderInstance;
  }  

  start = ( 
    sampleRate = 48000, 
    minSeconds = 0, 
    numChannels= true, 
    applyFade = false) =>
    {
    RNSuperpowered.startRecord(sampleRate, minSeconds, numChannels, applyFade);
  }
  
  stop = () => {
    RNSuperpowered.stopRecord();
  } 
}


