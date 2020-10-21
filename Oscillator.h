#pragma once

#include "VoiceManager.h"

#include <cmath>

#define M_PI 3.14159265358979323846

struct OscVoiceState
{
  double lastBaseFreq = 0;
  double modFrequency = 0;
  double phasePosition = 0;
  double phaseIncrement = 0;
};

enum EWaveform
{
  kSineWave,
  kTriangleWave,
  kSquareWave,
  kSawtoothWave,
  kNumWaveforms
};

class Oscillator
{
public:
  Oscillator();
  ~Oscillator();

  void ProcessVoices(VoiceState* voices);

  void SetWaveform(EWaveform waveform);

private:
  EWaveform mWaveform = EWaveform::kTriangleWave;

  OscVoiceState mOscVoiceStates[MAX_NUM_VOICES];

  double GetSample(double phasePos);

  double GetModFrequency(double baseFreq);
};

