/* eslint */

import { 
  Platform, NativeModules, NativeEventEmitter, DeviceEventEmitter
} from 'react-native'

import * as JsonCommands from './json_commands';

const { RNSuperpowered } = NativeModules;

// Depending on platform (at least in this version of RN) we need either a NativeEventEmitter or DeviceEventEmitter.
const jsonEmitter = (Platform.OS == 'ios')
  ? new NativeEventEmitter(RNSuperpowered)
  : DeviceEventEmitter;  


  type CmdResponse = {
    json:string;
  };

  type CmdResponseParsed = {
    rsp:string;
    rspData?: Object;
  };

//------------------------------------------------------------------------------
export class SuperpoweredApi {
  private static superpoweredInst:SuperpoweredApi;

  //eventListeners:any = {};
  //private eventListeners: Map<string, Array<(e:any)=>void>> = new Map();
  private eventListeners: { [key: string]: Array<(e:any)=>void> } = {};
  private localCommandId:number = 0; //< This increments with every issued command.
  private eventSubscription:any = null;
  private pendingCmdMap: { [key: string]: any } = {};

  constructor(){
    console.log("Creating Superpowered Class");

    this.eventSubscription = jsonEmitter.addListener('RnJsonEvent', (e)=>{
      console.log("Received JSON event: " + JSON.stringify(e));
      this._handleJsonEvent(e);
    });    
  }

  public static inst():SuperpoweredApi{
    if (!SuperpoweredApi.superpoweredInst){
      SuperpoweredApi.superpoweredInst = new SuperpoweredApi();
    }
    return SuperpoweredApi.superpoweredInst;
  }

  //------------------------------------------------------------------------------
  jsonCommand(commandStr:string, commandDataObject:Object = {}){

    let cmdData = (commandDataObject) ? commandDataObject : {};
    let cmdObj = {
      cmd:commandStr,
      cmdId: this._getCommandId(),
      cmdData:cmdData
    };

    let p = new Promise( (resolve, reject) => {
      let pending = {
        p:p,
        resolve:resolve,
        reject:reject,
        subscription:null
      }

      let pendingCmdId:string = cmdObj.cmdId.toString();
      this.pendingCmdMap[pendingCmdId] = pending;
  
      let str:string = JSON.stringify(cmdObj);
      RNSuperpowered.jsonCommand(str);
    } );
  
    return p;    
  }

  //------------------------------------------------------------------------------
  // Adds a listener for a specific response
  addListener( BleCommandsRsp:string, cb:(e:any)=>void ){
    if (BleCommandsRsp in this.eventListeners){
      this.eventListeners[BleCommandsRsp] = [...this.eventListeners[BleCommandsRsp], cb];
    }
    else {
      this.eventListeners[BleCommandsRsp] = [cb];
    }
  }

  //------------------------------------------------------------------------------
  removeListener( BleCommandsRsp:string, cb:(e:any)=>void ){
    if (BleCommandsRsp in this.eventListeners){
      let oldEventListeners:Array<(e:any)=>void> = this.eventListeners[BleCommandsRsp];
      let newEventListeners:Array<(e:any)=>void> = [];
      oldEventListeners.forEach((cmpCb:(e:any)=>void, index:number)=>{
        if (cb != cmpCb){
          newEventListeners = [...newEventListeners, cmpCb];
        }
      });
    }
  }

  //------------------------------------------------------------------------------
  _getCommandId():number {
    this.localCommandId = (0xff & (this.localCommandId+1));
    return this.localCommandId+1;
  }

  //------------------------------------------------------------------------------
  _handleJsonDispatch(robj:CmdResponseParsed){
    console.log("Got response " + robj.rsp);
    let rval:CmdResponseParsed = {
      rsp: robj.rsp
    };

    if ((robj.rsp) && (robj.rspData) && (robj.rsp in this.eventListeners)) {
      var listenerArray = this.eventListeners[robj.rsp];
      var x:{ [key: string]: any } = {};
      x[robj.rsp] = {...robj.rspData};
      rval.rspData = {...x};
      rval = {...rval, ...robj};
      listenerArray.map(function(cb){
        cb(rval);
      });
    }
    else {
      console.log('Unhandled event:' + robj.rsp);
    }
  }


  //------------------------------------------------------------------------------
  _handleJsonEvent(e:CmdResponse){
    let robj = JSON.parse(e.json);
    //let robjStr = JSON.stringify(robj);
    let pendingCmd = null;
    if ((!robj.cmdId) || (robj.cmdId <= 0)) {
      if (robj.rsp){
        this._handleJsonDispatch(robj);
      }
      else {
        // Command was spureously generated, so is an event, not a response.
        console.log("got spureous json event:" + JSON.stringify(robj));
      }
    }
    else {
      let pendingCmdId = robj.cmdId.toString();
      pendingCmd = this.pendingCmdMap[pendingCmdId];
      if (pendingCmd){
        //jsonEmitter.removeListener(pendingCmd.subscription);
        delete this.pendingCmdMap[pendingCmdId];
        let cmdRsp = (robj.cmdRsp) ? robj.cmdRsp : {};
        if (robj.status){
          if (pendingCmd.resolve){
            pendingCmd.resolve(cmdRsp);
          }
        }
        else {
          if (pendingCmd.reject){
            pendingCmd.reject(cmdRsp);
          }
        }
      }
    }
  }
}

export function InitializeSuperpowered(
  _licenseKey?: string,
  _enableAudioAnalysis: boolean = false,
  _enableFFTAndFrequencyDomain: boolean = false,
  _enableAudioTimeStretching: boolean = true,
  _enableAudioEffects: boolean = false,
  _enableAudioPlayerAndDecoder: boolean = true,
  _enableCryptographics: boolean = false,
  _enableNetworking: boolean = true,
){
  //CmdInitializeSuperpowered
  let dataObj: JsonCommands.CmdInitializeSuperpowered = {
    licenseKey: ((_licenseKey) && (_licenseKey.length > 2)) ? _licenseKey : '',
    enableAudioAnalysis: _enableAudioAnalysis,
    enableFFTAndFrequencyDomain: _enableFFTAndFrequencyDomain,
    enableAudioTimeStretching: _enableAudioTimeStretching,
    enableAudioEffects: _enableAudioEffects,
    enableAudioPlayerAndDecoder: _enableAudioPlayerAndDecoder,
    enableCryptographics: _enableCryptographics,
    enableNetworking: _enableNetworking,
  };
  
  SuperpoweredApi.inst().jsonCommand(
    JsonCommands.cmd_initialize_superpowered, 
    dataObj);  
}

/*
//------------------------------------------------------------
export function GenerateTone(
  freq:number, scale:number
){
  let tone: JsonCommands.CmdGenTone = {
    freq:freq,
    scale:scale
  };
  
  SuperpoweredApi.inst().jsonCommand(
    JsonCommands.cmd_gen_tone, 
    tone);
}

//------------------------------------------------------------
export function StartRecording(
  filePath: string,
  sampleRate: number,
  seconds: number,
){
  let dataObj: JsonCommands.CmdStartRecording = {
    filePath: filePath,
    sampleRate: sampleRate,
    seconds: seconds,
  };
  
  SuperpoweredApi.inst().jsonCommand(
    JsonCommands.cmd_start_recording, 
    dataObj);
}

//------------------------------------------------------------
export function StopRecording(){
  let dataObj: JsonCommands.CmdStopRecording = {
    dummy: "",
  };
  
  SuperpoweredApi.inst().jsonCommand(
    JsonCommands.cmd_stop_recording, 
    dataObj);
}
*/

//------------------------------------------------------------
export function StartPlayback(filePath:string){
  let dataObj: JsonCommands.CmdStartPlayback = {
    filePath: filePath
  };
  
  SuperpoweredApi.inst().jsonCommand(
    JsonCommands.cmd_start_playback, 
    dataObj);
}

//------------------------------------------------------------
export function StopPlayback(){
  let dataObj: JsonCommands.CmdStopPlayback = {
  };
  
  SuperpoweredApi.inst().jsonCommand(
    JsonCommands.cmd_stop_playback, 
    dataObj);
}

//------------------------------------------------------------
export function SetDirection(reverse:boolean){
  let dataObj: JsonCommands.CmdSetPlaybackDirection = {
    reverse: reverse,
  };
  
  SuperpoweredApi.inst().jsonCommand(
    JsonCommands.cmd_set_playback_direction, 
    dataObj);
}

//------------------------------------------------------------
export function SetSpeed(speed:number){
  let dataObj: JsonCommands.CmdSetPlaybackSpeed = {
    speed: speed,
  };
  
  SuperpoweredApi.inst().jsonCommand(
    JsonCommands.cmd_set_playback_speed, 
    dataObj);
}
