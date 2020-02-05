
export const cmd_invalid:string = 'cmd_invalid';
export const cmd_gen_tone:string = 'cmd_gen_tone';
export const cmd_start_recording:string = 'cmd_start_recording';
export const cmd_stop_recording:string = 'cmd_stop_recording';
export const cmd_start_playback:string = 'cmd_start_playback';
export const cmd_set_playback_speed:string = 'cmd_set_playback_speed';
export const cmd_set_playback_direction:string = 'cmd_set_playback_direction';


// cmd_gen_tone
export type CmdGenTone = {
  freq:number,
  scale:number, 
};

export type CmdStartRecording = {
  filePath: string,
  sampleRate: number,
  seconds: number,
};

export type CmdStopRecording = {
  // nothing
  dummy:string,
};

export type CmdStartPlayback = {
  filePath: string;
};

export type CmdSetPlaybackSpeed = {
  speed: number; // -20..0..20
};

export type CmdSetPlaybackDirection = {
  reverse: boolean;
};
