#include "OscMixer.h"

OscMixer::OscMixer(int numOscillators)
{
  for (int i = 0; i < numOscillators; i++) {
    mLevels.push_back(0.0);
  }
}

void OscMixer::ProcessVoices(VoiceState* voices)
{
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    voices[i].sampleValue = 0;
    for (int j = 0; j < mOscillators.size(); j++)
    {
      voices[i].sampleValue += mOscillators[j]->GetSampleValue(i) * mLevels[j];
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
