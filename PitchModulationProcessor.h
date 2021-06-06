#pragma once

#include "VoiceManager.h"
#include "BigBlueAudioModule.h"

#include <cmath>

const static double SEMITONE = pow(2.0, 1.0 / 12.0);
static double GetPitchMultiplier(double semitones);
static double GetSemitones(double pitchMultiplier);

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
  kPortamentoModeAlways,
  kPortamentoModeLegato,
  kPortamentoModeNever,
};

enum EPortamentoType
{
  kPortamentoTypeRate,
  kPortamentoTypeTime,
};

const std::initializer_list<const char*> PORTAMENTO_MODE_NAMES =
{
  "Always", "Legato", "Never"
};

const std::initializer_list<const char*> PORTAMENTO_TYPE_NAMES =
{
  "Rate", "Time"
};

class PortamentoProcessor : public BigBlueAudioModule
{
public:
  PortamentoProcessor();
  ~PortamentoProcessor();

  void ProcessVoices(VoiceState* voices);

  void SetPortamentoMode(EPortamentoMode mode);
  void SetPortamentoType(EPortamentoType type);
  void SetPortamentoTime(double seconds);
  void SetPortamentoRate(double semitonesPerSecond);

private:
  EPortamentoMode mCurrentMode = kPortamentoModeNever;
  EPortamentoType mCurrentType = kPortamentoTypeTime;
  double mPortamentoTime = 0; // in seconds
  double mPortamentoRate = 0; // in seconds per semitone

  double mRawVoiceFrequencies[MAX_NUM_VOICES];
  int mVoiceOrderPressed[MAX_NUM_VOICES];

  void InitializeVoicePortamento(VoiceState* voice, PortamentoVoiceState* portVoiceState, double startFrequency);

  void TrackNoteStartOrder(int voiceIdx, double frequency);
  double GetPortamentoStartFrequency(VoiceState* voices);

  PortamentoVoiceState mPortVoiceStates[MAX_NUM_VOICES];
};

