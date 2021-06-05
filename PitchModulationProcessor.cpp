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

void PitchModulationProcessor::ProcessIndividualModulation(VoiceState* voices)
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
  mCurrentGlobalMultiplier = GetPitchMultiplier(mCurrentGlobalMod);
  // Possibly TODO later: smoothing functionality
}

inline double PitchModulationProcessor::GetPitchMultiplier(double semitones)
{
  return pow(SEMITONE, semitones);
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
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (voices[i].isSounding)
    {
      if (voices[i].event == EVoiceEvent::kNoteStart)
      {

        // TODO: determine these values
        double totalModulation = 12; // in semitones
        mPitchVoiceStates[i].isBending = true;

        // Set the start, current, and target frequencies
        mPitchVoiceStates[i].startFreq = voices[i].frequency
          / GetPitchMultiplier(totalModulation);
        mPitchVoiceStates[i].currentFreq = mPitchVoiceStates[i].startFreq;
        mPitchVoiceStates[i].targetFreq = voices[i].frequency;

        // Get modulation time
        double durationTime;
        if (mIsModeTime)
          durationTime = mPortamentoTime;
        else
          durationTime = mPortamentoRate * totalModulation;
        mPitchVoiceStates[i].samplesRemaining = durationTime * SampleRate();

        // Calculate increment per sample
        double semitonesPerSample = totalModulation / mPitchVoiceStates[i].samplesRemaining;
        mPitchVoiceStates[i].deltaFreq = GetPitchMultiplier(semitonesPerSample);

        // Send the start frequency back to the voice
        voices[i].frequency = mPitchVoiceStates[i].currentFreq;
      }
      else if (mPitchVoiceStates[i].isBending)
      {
        // This block is only reached if the voice is on its 2nd (or later)
        // sample of portamento

        // Increment (multipicatively) the frequecy and update sample timer
        mPitchVoiceStates[i].currentFreq *= mPitchVoiceStates[i].deltaFreq;
        voices[i].frequency = mPitchVoiceStates[i].currentFreq;
        mPitchVoiceStates[i].samplesRemaining--;

        // Check if the bend is over
        if (mPitchVoiceStates[i].samplesRemaining == 0)
        {
          // Adjust for any accumulated floating point errors
          // In testing, this is usually accurate to around 10 significant figures
          mPitchVoiceStates[i].currentFreq = mPitchVoiceStates[i].targetFreq;
          voices[i].frequency = mPitchVoiceStates[i].currentFreq;

          // Turn off
          mPitchVoiceStates[i].isBending = false;
        }
      }
    }
  }
}

void PortamentoProcessor::SetPortamentoTime(double seconds)
{
  mPortamentoTime = seconds;
  mIsModeTime = true;
}

void PortamentoProcessor::SetPortamentoRate(double secondsPerSemitone)
{
  mPortamentoRate = secondsPerSemitone;
  mIsModeTime = false;
}
