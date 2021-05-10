#include "BigBlueAudioModule.h"

BigBlueAudioModule::BigBlueAudioModule()
{
}

void BigBlueAudioModule::SetSampleRate(double sampleRate)
{
  mSampleRate = sampleRate;
  
  mSecondsPerSample = 1.0 / sampleRate;
}

double BigBlueAudioModule::SampleRate()
{
  return mSampleRate;
}

double BigBlueAudioModule::SecondsPerSample()
{
  return mSecondsPerSample;
}
