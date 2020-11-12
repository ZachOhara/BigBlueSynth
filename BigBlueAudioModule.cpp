#include "BigBlueAudioModule.h"

BigBlueAudioModule::BigBlueAudioModule()
{
}

void BigBlueAudioModule::SetSampleRate(double sampleRate)
{
  mSampleRate = sampleRate;
}

double BigBlueAudioModule::SampleRate()
{
  return mSampleRate;
}
