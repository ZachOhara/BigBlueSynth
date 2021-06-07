#include "OscMixer.h"

OscMixer::OscMixer(int numOscillators)
{
  for (int i = 0; i < numOscillators; i++) {
    mLevels.push_back(0.0);
  }
}

void OscMixer::ProcessVoices(VoiceState* voices)
{
  int nOscs = mOscillators.size();
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (voices[i].isSounding)
    {
      voices[i].sampleValue = 0;
      for (int j = 0; j < nOscs; j++)
      {
        voices[i].sampleValue += mOscillators[j]->GetSampleValue(i) * mLevels[j];
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
