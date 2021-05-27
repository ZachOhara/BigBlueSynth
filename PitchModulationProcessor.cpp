#include "PitchModulationProcessor.h"

PitchModulationProcessor::PitchModulationProcessor()
{
}

PitchModulationProcessor::~PitchModulationProcessor()
{
}

void PitchModulationProcessor::ProcessGlobalModulation(VoiceState* voices)
{
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (voices[i].isSounding)
    {
      // Determine if the frequency has been modified by another module
      if (voices[i].frequency != mPitchVoiceStates[i].lastFreq)
      {
        mPitchVoiceStates[i].initialFreq = voices[i].frequency;
      }
      // Apply global modulation
      voices[i].frequency = mPitchVoiceStates[i].initialFreq * mCurrentGlobalMultiplier;
      mPitchVoiceStates[i].lastFreq = voices[i].frequency;
    }
  }
}

void PitchModulationProcessor::SetGlobalModulation(double semitones)
{
  mCurrentGlobalMod = semitones;
  mCurrentGlobalMultiplier = pow(SEMITONE, mCurrentGlobalMod);
  // Possibly TODO later: smoothing functionality
}

PitchWheelProcessor::PitchWheelProcessor()
{
}

PitchWheelProcessor::~PitchWheelProcessor()
{
}

void PitchWheelProcessor::ProcessVoices(VoiceState* voices)
{
  ProcessGlobalModulation(voices);
}

void PitchWheelProcessor::SetWheelPosition(double position)
{
  // position should be a value on [-1, 1]
  SetGlobalModulation(mSemitoneRange * position);
}

void PitchWheelProcessor::SetSemitomeRange(int semitones)
{
  mSemitoneRange = semitones;
}

PortamentoProcessor::PortamentoProcessor()
{
}

PortamentoProcessor::~PortamentoProcessor()
{
}

void PortamentoProcessor::ProcessVoices(VoiceState* voices)
{
}

void PortamentoProcessor::SetPortamentoTime(double seconds)
{
}

void PortamentoProcessor::SetPortamentoRate(double semitonesPerSecond)
{
}
