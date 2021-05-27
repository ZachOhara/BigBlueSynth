#pragma once

#include "VoiceManager.h"
#include "BigBlueAudioModule.h"

#include <cmath>

struct PitchVoiceState
{
  double initialFreq = 0; // the baseline frequency before this processor modifies it
  double lastFreq = 0; // used to dermine it something elso has modified the note
  //double targetFreq;
  //double deltaFreq;
};

class PitchModulationProcessor : public BigBlueAudioModule
{
public:

  PitchModulationProcessor();
  ~PitchModulationProcessor();

  void ProcessGlobalModulation(VoiceState* voices);

protected:
  void SetGlobalModulation(double semitones);

private:
  const double SEMITONE = pow(2.0, 1.0 / 12.0);

  // Current semitone modulation is tracked in case smoothing needs to be added later
  double mCurrentGlobalMod = 0; // in semitones
  double mCurrentGlobalMultiplier = 1;

  PitchVoiceState mPitchVoiceStates[MAX_NUM_VOICES];
};

class PitchWheelProcessor : public PitchModulationProcessor
{
public:
  PitchWheelProcessor();
  ~PitchWheelProcessor();

  void ProcessVoices(VoiceState* voices);

  void SetWheelPosition(double position);
  void SetSemitomeRange(int semitones);

private:
  int mSemitoneRange = 2; // Default to a whole step of range

};

class PortamentoProcessor : public PitchModulationProcessor
{
public:
  PortamentoProcessor();
  ~PortamentoProcessor();

  void ProcessVoices(VoiceState* voices);

  void SetPortamentoTime(double seconds);
  void SetPortamentoRate(double semitonesPerSecond);

private:
  double mPortamentoTime;
  double mPortamentoRate; 
};

