#pragma once

#include "Oscillator.h"
#include "VoiceManager.h"
#include "BigBlueAudioModule.h"

#include <vector>

class OscMixer : public BigBlueAudioModule
{
public:
  OscMixer(int numOscillators);

  void ProcessVoices(VoiceState* voices);

  void AddOscillator(Oscillator* oscillator);
  void SetMixLevel(int oscIndex, double level);

private:
  std::vector<Oscillator*> mOscillators;
  std::vector<double> mLevels;
};
