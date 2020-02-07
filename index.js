/* eslint */
var react_native_1 = require('react-native');
var JsonCommands = require('./json_commands');
var RNSuperpowered = react_native_1.NativeModules.RNSuperpowered;
// Depending on platform (at least in this version of RN) we need either a NativeEventEmitter or DeviceEventEmitter.
var jsonEmitter = (react_native_1.Platform.OS == 'ios')
    ? new react_native_1.NativeEventEmitter(RNSuperpowered)
    : react_native_1.DeviceEventEmitter;
//------------------------------------------------------------------------------
var SuperpoweredApi = (function () {
    function SuperpoweredApi() {
        var _this = this;
        //eventListeners:any = {};
        //private eventListeners: Map<string, Array<(e:any)=>void>> = new Map();
        this.eventListeners = {};
        this.localCommandId = 0; //< This increments with every issued command.
        this.eventSubscription = null;
        this.pendingCmdMap = {};
        console.log("Creating Superpowered Class");
        this.eventSubscription = jsonEmitter.addListener('RnJsonEvent', function (e) {
            console.log("Received JSON event: " + JSON.stringify(e));
            _this._handleJsonEvent(e);
        });
    }
    SuperpoweredApi.inst = function () {
        if (!SuperpoweredApi.superpoweredInst) {
            SuperpoweredApi.superpoweredInst = new SuperpoweredApi();
        }
        return SuperpoweredApi.superpoweredInst;
    };
    //------------------------------------------------------------------------------
    SuperpoweredApi.prototype.jsonCommand = function (commandStr, commandDataObject) {
        var _this = this;
        if (commandDataObject === void 0) { commandDataObject = {}; }
        var cmdData = (commandDataObject) ? commandDataObject : {};
        var cmdObj = {
            cmd: commandStr,
            cmdId: this._getCommandId(),
            cmdData: cmdData
        };
        var p = new Promise(function (resolve, reject) {
            var pending = {
                p: p,
                resolve: resolve,
                reject: reject,
                subscription: null
            };
            var pendingCmdId = cmdObj.cmdId.toString();
            _this.pendingCmdMap[pendingCmdId] = pending;
            var str = JSON.stringify(cmdObj);
            RNSuperpowered.jsonCommand(str);
        });
        return p;
    };
    //------------------------------------------------------------------------------
    // Adds a listener for a specific response
    SuperpoweredApi.prototype.addListener = function (BleCommandsRsp, cb) {
        if (BleCommandsRsp in this.eventListeners) {
            this.eventListeners[BleCommandsRsp] = this.eventListeners[BleCommandsRsp].concat([cb]);
        }
        else {
            this.eventListeners[BleCommandsRsp] = [cb];
        }
    };
    //------------------------------------------------------------------------------
    SuperpoweredApi.prototype.removeListener = function (BleCommandsRsp, cb) {
        if (BleCommandsRsp in this.eventListeners) {
            var oldEventListeners = this.eventListeners[BleCommandsRsp];
            var newEventListeners = [];
            oldEventListeners.forEach(function (cmpCb, index) {
                if (cb != cmpCb) {
                    newEventListeners = newEventListeners.concat([cmpCb]);
                }
            });
        }
    };
    //------------------------------------------------------------------------------
    SuperpoweredApi.prototype._getCommandId = function () {
        this.localCommandId = (0xff & (this.localCommandId + 1));
        return this.localCommandId + 1;
    };
    //------------------------------------------------------------------------------
    SuperpoweredApi.prototype._handleJsonDispatch = function (robj) {
        console.log("Got response " + robj.rsp);
        var rval = {
            rsp: robj.rsp
        };
        if ((robj.rsp) && (robj.rspData) && (robj.rsp in this.eventListeners)) {
            var listenerArray = this.eventListeners[robj.rsp];
            var x = {};
            x[robj.rsp] = { robj: .rspData };
            rval.rspData = { x: x };
            rval = { rval: rval, robj: robj };
            listenerArray.map(function (cb) {
                cb(rval);
            });
        }
        else {
            console.log('Unhandled event:' + robj.rsp);
        }
    };
    //------------------------------------------------------------------------------
    SuperpoweredApi.prototype._handleJsonEvent = function (e) {
        var robj = JSON.parse(e.json);
        //let robjStr = JSON.stringify(robj);
        var pendingCmd = null;
        if ((!robj.cmdId) || (robj.cmdId <= 0)) {
            if (robj.rsp) {
                this._handleJsonDispatch(robj);
            }
            else {
                // Command was spureously generated, so is an event, not a response.
                console.log("got spureous json event:" + JSON.stringify(robj));
            }
        }
        else {
            var pendingCmdId = robj.cmdId.toString();
            pendingCmd = this.pendingCmdMap[pendingCmdId];
            if (pendingCmd) {
                //jsonEmitter.removeListener(pendingCmd.subscription);
                delete this.pendingCmdMap[pendingCmdId];
                var cmdRsp = (robj.cmdRsp) ? robj.cmdRsp : {};
                if (robj.status) {
                    if (pendingCmd.resolve) {
                        pendingCmd.resolve(cmdRsp);
                    }
                }
                else {
                    if (pendingCmd.reject) {
                        pendingCmd.reject(cmdRsp);
                    }
                }
            }
        }
    };
    return SuperpoweredApi;
})();
exports.SuperpoweredApi = SuperpoweredApi;
function InitializeSuperpowered(_licenseKey, _enableAudioAnalysis, _enableFFTAndFrequencyDomain, _enableAudioTimeStretching, _enableAudioEffects, _enableAudioPlayerAndDecoder, _enableCryptographics, _enableNetworking) {
    if (_enableAudioAnalysis === void 0) { _enableAudioAnalysis = false; }
    if (_enableFFTAndFrequencyDomain === void 0) { _enableFFTAndFrequencyDomain = false; }
    if (_enableAudioTimeStretching === void 0) { _enableAudioTimeStretching = true; }
    if (_enableAudioEffects === void 0) { _enableAudioEffects = false; }
    if (_enableAudioPlayerAndDecoder === void 0) { _enableAudioPlayerAndDecoder = true; }
    if (_enableCryptographics === void 0) { _enableCryptographics = false; }
    if (_enableNetworking === void 0) { _enableNetworking = true; }
    //CmdInitializeSuperpowered
    var dataObj = {
        licenseKey: ((_licenseKey) && (_licenseKey.length > 2)) ? _licenseKey : '',
        enableAudioAnalysis: _enableAudioAnalysis,
        enableFFTAndFrequencyDomain: _enableFFTAndFrequencyDomain,
        enableAudioTimeStretching: _enableAudioTimeStretching,
        enableAudioEffects: _enableAudioEffects,
        enableAudioPlayerAndDecoder: _enableAudioPlayerAndDecoder,
        enableCryptographics: _enableCryptographics,
        enableNetworking: _enableNetworking
    };
    SuperpoweredApi.inst().jsonCommand(JsonCommands.cmd_initialize_superpowered, dataObj);
}
exports.InitializeSuperpowered = InitializeSuperpowered;
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
function StartPlayback(filePath) {
    var dataObj = {
        filePath: filePath
    };
    SuperpoweredApi.inst().jsonCommand(JsonCommands.cmd_start_playback, dataObj);
}
exports.StartPlayback = StartPlayback;
//------------------------------------------------------------
function StopPlayback() {
    var dataObj = {};
    SuperpoweredApi.inst().jsonCommand(JsonCommands.cmd_stop_playback, dataObj);
}
exports.StopPlayback = StopPlayback;
//------------------------------------------------------------
function SetDirection(reverse) {
    var dataObj = {
        reverse: reverse
    };
    SuperpoweredApi.inst().jsonCommand(JsonCommands.cmd_set_playback_direction, dataObj);
}
exports.SetDirection = SetDirection;
//------------------------------------------------------------
function SetSpeed(speed) {
    var dataObj = {
        speed: speed
    };
    SuperpoweredApi.inst().jsonCommand(JsonCommands.cmd_set_playback_speed, dataObj);
}
exports.SetSpeed = SetSpeed;
