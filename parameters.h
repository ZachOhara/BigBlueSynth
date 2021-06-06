#pragma once

enum EParams
{
  // General settings
  kActiveVoices,
  // Portamento
  kPortamentoMode,
  kPortamentoType,
  kPortamentoTime,
  kPortamentoRate,
  // Oscillators
  kOsc1OctavePid,
  kOsc1WaveformPid,
  kOsc1SemitonePid,
  kOsc1DetunePid,
  kOsc2OctavePid,
  kOsc2WaveformPid,
  kOsc2SemitonePid,
  kOsc2DetunePid,
  // Osc mixer
  kMixLevelOsc1,
  kMixLevelOsc2,
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
