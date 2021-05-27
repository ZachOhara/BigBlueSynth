#pragma once

#include "VoiceManager.h"
#include "BigBlueAudioModule.h"

#include <cmath>
#include <vector>

struct EnvVoiceState
{
  int currentSegment = 0; // see enum EnvelopeProcessor::ESegments
  int segmentSamples = 0; // number of samples elapsed in the current segment
  int remainingSamples = 0; // in the current segment
  //double segmentExponent = 1.0; // maybe exponents will be supported later?
  double segmentInitialOutput = 0; // loudness when this segment was started, on [0,1]
  double segmentDifference = 0; // change in loudnsss that will occur this segment
  double currentOutput = 0; // the current sample's loudness
};


class EnvelopeProcessor : public BigBlueAudioModule
{
public:
  EnvelopeProcessor();
  ~EnvelopeProcessor();

  void ProcessVoices(VoiceState* voices);

  void SetAttackTime(double millis);
  void SetDecayTime(double millis);
  void SetSustainLevel(double loudness);
  void SetReleaseTime(double millis);
  void SetPeakLevel(double loudness);

private:
  enum ESegments { kSilenceSegment, kAttackSegment, kDecaySegment, kSustainSegment, kReleaseSegment };

  double mAttackTime; // in seconds
  double mDecayTime;
  double mSustainLevel; // in loudness on [0,1]
  double mReleaseTime;
  double mPeakLevel;

  bool IsInStationarySegment(EnvVoiceState* voice);
  void ProgressToSegment(EnvVoiceState* voice, int newSegment, double duration, double goal);

  EnvVoiceState mEnvVoiceStates[MAX_NUM_VOICES];
};

