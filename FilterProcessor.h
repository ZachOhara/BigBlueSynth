#pragma once

#include "VoiceManager.h"
#include "BigBlueAudioModule.h"

#include <cmath>
#include <vector>

#define KERNEL_SIZE 64 // Todo: experiment with this value


// This is being implemented for when we eventually add filter contour
struct FilterVoiceState
{
  double lastSamples[KERNEL_SIZE];
  int placeIndex = 0; // where the next sample should be placed
};



class FilterProcessor : public BigBlueAudioModule
{
public:
  FilterProcessor();
  ~FilterProcessor();

  void ProcessVoices(VoiceState* voices);

  void SetCutoffFrequency(double hz);
  void SetResonance(double resonance);

private:
  enum ESegments { kSilenceSegment,  kAttackSegment, kDecaySegment, kSustainSegment, kReleaseSegment };

  double mCutoffFrequency; // in hz
  double mResonance; // normalized on [0,1]

  double mFilterKernel[KERNEL_SIZE];

  void RecalculateFilterKernel();
  void ApplyKernelWindow();

  void FFT(double* in, double* out);
  void IFFT(double* in, double* out);

  FilterVoiceState mFilterVoiceStates[MAX_NUM_VOICES];
};

