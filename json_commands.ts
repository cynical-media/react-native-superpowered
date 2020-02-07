
/* eslint-disable prettier/prettier */

export const cmd_invalid:string = 'cmd_invalid';
export const cmd_initialize_superpowered:string = 'cmd_initialize_superpowered';
//export const cmd_gen_tone:string = 'cmd_gen_tone';
//export const cmd_start_recording:string = 'cmd_start_recording';
//export const cmd_stop_recording:string = 'cmd_stop_recording';
export const cmd_start_playback:string = 'cmd_start_playback';
export const cmd_stop_playback:string = 'cmd_stop_playback';
export const cmd_set_playback_speed:string = 'cmd_set_playback_speed';
export const cmd_set_playback_direction:string = 'cmd_set_playback_direction';

// cmd_superpowered_initialize
/// @fn Initialize(const char *licenseKey, bool enableAudioAnalysis, bool enableFFTAndFrequencyDomain, bool enableAudioTimeStretching, bool enableAudioEffects, bool enableAudioPlayerAndDecoder, bool enableCryptographics, bool enableNetworking);
/// @brief Initializes the Superpowered SDKs. Use this only once, when your app or library initializes.
/// Do not use this if Superpowered is loaded dynamically and might be loaded multiple times (a DLL in a VST host for example). @see DynamicInitialize
/// @param licenseKey Visit https://superpowered.com/dev to register license keys.
/// @param enableAudioAnalysis Enables Analyzer, LiveAnalyzer, Waveform and BandpassFilterbank.
/// @param enableFFTAndFrequencyDomain Enables FrequencyDomain, FFTComplex, FFTReal and PolarFFT.
/// @param enableAudioTimeStretching Enables TimeStretching.
/// @param enableAudioEffects Enables all effects and every class based on the FX class.
/// @param enableAudioPlayerAndDecoder Enables AdvancedAudioPlayer and Decoder.
/// @param enableCryptographics Enables RSAPublicKey,RSAPrivateKey, hasher and AES.
/// @param enableNetworking Enables httpRequest.
export type CmdInitializeSuperpowered  = {
  licenseKey?: string;
  enableAudioAnalysis: boolean;
  enableFFTAndFrequencyDomain: boolean;
  enableAudioTimeStretching: boolean;
  enableAudioEffects: boolean;
  enableAudioPlayerAndDecoder: boolean;
  enableCryptographics: boolean;
  enableNetworking: boolean;
};


// cmd_gen_tone
//export type CmdGenTone = {
//  freq:number,
//  scale:number, 
//};

//export type CmdStartRecording = {
//  filePath: string,
//  sampleRate: number,
//  seconds: number,
//};

//export type CmdStopRecording = {
//  // nothing
//  dummy:string,
//};

export type CmdStartPlayback = {
  filePath: string;
};

export type CmdStopPlayback = {
};

export type CmdSetPlaybackSpeed = {
  speed: number; // -20..0..20
};

export type CmdSetPlaybackDirection = {
  reverse: boolean;
};
