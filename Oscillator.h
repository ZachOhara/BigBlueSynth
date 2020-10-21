#pragma once

#include "VoiceManager.h"

#include <cmath>
#include <vector>

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

const std::initializer_list<const char*> WAVEFORM_NAMES = { "Sine", "Triangle", "Square", "Saw" };
const std::initializer_list<const char*> OCTAVE_NAMES = { "−  1", "+ 0", "+  1", "+ 2" };

class Oscillator
{
public:
  Oscillator();
  ~Oscillator();

  void ProcessVoices(VoiceState* voices);

  void SetOctaveMod(int octaveMod);
  void SetWaveform(EWaveform waveform);

private:
  int mOctaveMod = 0;
  EWaveform mWaveform = EWaveform::kTriangleWave;

  OscVoiceState mOscVoiceStates[MAX_NUM_VOICES];

  double GetSample(double phasePos);

  double GetModFrequency(double baseFreq);
};

