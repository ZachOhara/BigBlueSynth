#pragma once

#include "VoiceManager.h"
#include "BigBlueAudioModule.h"

#include <cmath>
#include <vector>

#define M_PI 3.14159265358979323846

struct OscVoiceState
{
  double lastBaseFreq = 0;
  double modFrequency = 0;
  double phasePosition = 0;
  double phaseIncrement = 0;
  double oscSampleValue = 0;
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

class Oscillator : public BigBlueAudioModule
{
public:
  Oscillator();
  ~Oscillator();

  virtual void ProcessVoices(VoiceState* voices);

  virtual double GetSampleValue(int voiceIdx);

  void SetWaveform(EWaveform waveform);
  void SetOctaveMod(double octaveMod);
  void SetSemitoneMod(double semitoneMod);
  void SetCentsMod(double centsMod);

private:
  EWaveform mWaveform;
  double mOctaveMod = 0;
  double mSemitoneMod = 0;
  double mCentsMod = 0;

  double mCumulFreqMod = 0;
  bool mIsModDirty = false;

  OscVoiceState mOscVoiceStates[MAX_NUM_VOICES];

  double GetSample(double phasePos);

  void RecalculateMods();
  inline double GetModFrequency(double baseFreq);
};

enum ESubWaveform
{
  kSubSquareWave,
  kSubSawtoothWave,
  kNumSubWaveforms
};

const std::initializer_list<const char*> SUB_WAVEFORM_NAMES = { "Square", "Saw" };
const std::initializer_list<const char*> SUB_OCTAVE_NAMES = { "Bass", "Rumble" };

class SubOscillator : public Oscillator
{
public:
  SubOscillator();
  ~SubOscillator();

  virtual void ProcessVoices(VoiceState* voices) override;
  virtual double GetSampleValue(int voiceIdx) override;

  virtual void HandleReset() override;

  void SetSubWaveform(ESubWaveform waveform);
  void SetLowOctaveBlend(double blend);

private:
  double mLowOctaveBlend;
  Oscillator mLowOctaveOsc;
};

