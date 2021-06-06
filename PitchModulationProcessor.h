#pragma once

#include "VoiceManager.h"
#include "BigBlueAudioModule.h"

#include <cmath>

static double GetPitchMultiplier(double semitones);

struct PitchVoiceState
{
  double initialFreq = 0; // the baseline frequency before this processor modifies it
  double lastFreq = 0; // used to dermine it something elso has modified the note
};

class PitchWheelProcessor : public BigBlueAudioModule
{
public:
  PitchWheelProcessor();
  ~PitchWheelProcessor();

  void ProcessVoices(VoiceState* voices);

  void SetWheelPosition(double position);
  void SetSemitomeRange(int semitones);

private:
  int mSemitoneRange = 2; // Default to a whole step of range

  // Current semitone modulation is tracked in case smoothing needs to be added later
  double mCurrentGlobalMod = 0; // in semitones
  double mCurrentGlobalMultiplier = 1;

  void SetModulation(double semitones);

  PitchVoiceState mPitchVoiceStates[MAX_NUM_VOICES];
};

struct PortamentoVoiceState
{
  bool isBending = false;
  double startFreq = 0;
  double targetFreq = 0;
  double currentFreq = 0;
  double deltaFreq = 0; // this is a multiplicative delta, not additive
  int samplesRemaining = 0;
};

enum EPortamentoMode
{
  kPortamentoModeRate,
  kPortamentoModeTime,
  kPortamentoModeOff,
};

const std::initializer_list<const char*> PORTAMENTO_MODE_NAMES =
{
  "Rate",
  "Time",
  "Off",
};

class PortamentoProcessor : public BigBlueAudioModule
{
public:
  PortamentoProcessor();
  ~PortamentoProcessor();

  void ProcessVoices(VoiceState* voices);

  void SetPortamentoMode(EPortamentoMode mode);
  void SetPortamentoTime(double seconds);
  void SetPortamentoRate(double semitonesPerSecond);

private:
  EPortamentoMode mCurrentMode = kPortamentoModeOff;
  double mPortamentoTime = 2.0; // in seconds
  double mPortamentoRate = 0; // in seconds per semitone

  PortamentoVoiceState mPortVoiceStates[MAX_NUM_VOICES];
};

