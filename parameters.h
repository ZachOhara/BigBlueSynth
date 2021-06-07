#pragma once

enum EParams
{
  // Synth mode
  kSynthMode,
  kActiveVoices,
  // Portamento
  kPortamentoMode,
  kPortamentoType,
  kPortamentoTime,
  kPortamentoRate,
  // Vibrato
  kVibratoRatePid,
  kVibratoDepthPid,
  // Oscillators
  kOsc1OctavePid,
  kOsc1WaveformPid,
  kOsc1SemitonePid,
  kOsc1DetunePid,
  kOsc2OctavePid,
  kOsc2WaveformPid,
  kOsc2SemitonePid,
  kOsc2DetunePid,
  kOscSubOctavePid,
  kOscSubWaveformPid,
  // Osc mixer
  kMixLevelOsc1,
  kMixLevelOsc2,
  kMixLevelSub,
  // Envelope
  kEnvAttackPid,
  kEnvDecayPid,
  kEnvSustainPid,
  kEnvReleasePid,
  kEnvPeakPid,
  // Filter
  kFilCutoffPid,
  kNumParams
};
