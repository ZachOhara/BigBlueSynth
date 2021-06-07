#include "OscMixer.h"

OscMixer::OscMixer(int numOscillators)
{
  for (int i = 0; i < numOscillators; i++) {
    mLevels.push_back(0.0);
  }
}

void OscMixer::ProcessVoices(VoiceState* voices)
{
  // Clear out last sample
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    voices[i].sampleValue = 0;
  }

  // Calculate the current sample
  int nOscs = mOscillators.size();
  double level = 0;
  Oscillator* osc = nullptr;
  // Normally the voices are the first-order iteration, but this is way faster
  // to do the oscillators first, since we can cache the values
  for (int i = 0; i < nOscs; i++)
  {
    level = mLevels[i];
    if (level > 0) // If the level is zero, skip the oscillator
    {
      osc = mOscillators[i];
      for (int j = 0; j < MAX_NUM_VOICES; j++)
      {
        if (voices[j].isSounding)
        {
          voices[j].sampleValue += osc->GetSampleValue(j) * level;
        }
      }
    }
  }
}

void OscMixer::AddOscillator(Oscillator* oscillator)
{
  mOscillators.push_back(oscillator);
}

void OscMixer::SetMixLevel(int oscIndex, double level)
{
  mLevels[oscIndex] = level;
}
