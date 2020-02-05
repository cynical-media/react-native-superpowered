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


var superpoweredInst = null;

//------------------------------------------------------------------------------
export class SuperpoweredApi {

  //eventListeners:any = {};
  eventListeners: Map<string, Array<(e:any)=>void>> = new Map();
  localCommandId:number = 0; //< This increments with every issued command.
  eventSubscription:any = null;
  pendingCmdMap:Object = {};

  constructor(){
    console.log("Creating Superpowered Class");

    this.eventSubscription = jsonEmitter.addListener('RnJsonEvent', (e)=>{
      console.log("Received JSON event: " + JSON.stringify(e));
      this._handleJsonEvent(e);
    });    
  }

  static inst(){
    if (null == superpoweredInst){
      superpoweredInst = new SuperpoweredApi;
    }
    return superpoweredInst;
  }

  //------------------------------------------------------------------------------
  jsonCommand(commandStr, commandDataObject){

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
  _handleJsonDispatch(robj){
    console.log("Got response " + robj.rsp);
    var rval = {
      rsp: robj.rsp,
      rspData:null,
    };

    if ((robj.rsp) && (robj.rspData) && (robj.rsp in this.eventListeners)) {
      var listenerArray = this.eventListeners[robj.rsp];
      var x = {};
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
  _handleJsonEvent(e){
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
