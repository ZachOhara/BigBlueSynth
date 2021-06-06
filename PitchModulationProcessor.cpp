#include "PitchModulationProcessor.h"


static double GetPitchMultiplier(double semitones)
{
  const static double SEMITONE = pow(2.0, 1.0 / 12.0);
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

void PitchWheelProcessor::SetWheelPosition(double position)
{
  // Position should be a value on [-1, 1]
  SetModulation(mSemitoneRange * position);
}

void PitchWheelProcessor::SetSemitomeRange(int semitones)
{
  mSemitoneRange = semitones;
}

void PitchWheelProcessor::SetModulation(double semitones)
{
  mCurrentGlobalMod = semitones;
  mCurrentGlobalMultiplier = GetPitchMultiplier(mCurrentGlobalMod);
  // If smoothing functionality is ever implemented, it should go here
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
        mPortVoiceStates[i].isBending = true;

        // Set the start, current, and target frequencies
        mPortVoiceStates[i].startFreq = voices[i].frequency
          / GetPitchMultiplier(totalModulation);
        mPortVoiceStates[i].currentFreq = mPortVoiceStates[i].startFreq;
        mPortVoiceStates[i].targetFreq = voices[i].frequency;

        // Get modulation time
        double durationTime;
        if (mIsModeTime)
          durationTime = mPortamentoTime;
        else
          durationTime = mPortamentoRate * totalModulation;
        mPortVoiceStates[i].samplesRemaining = durationTime * SampleRate();

        // Calculate increment per sample
        double semitonesPerSample = totalModulation / mPortVoiceStates[i].samplesRemaining;
        mPortVoiceStates[i].deltaFreq = GetPitchMultiplier(semitonesPerSample);

        // Send the start frequency back to the voice
        voices[i].frequency = mPortVoiceStates[i].currentFreq;
      }
      else if (mPortVoiceStates[i].isBending)
      {
        // This block is only reached if the voice is on its 2nd (or later)
        // sample of portamento

        // Increment (multipicatively) the frequecy and update sample timer
        mPortVoiceStates[i].currentFreq *= mPortVoiceStates[i].deltaFreq;
        voices[i].frequency = mPortVoiceStates[i].currentFreq;
        mPortVoiceStates[i].samplesRemaining--;

        // Check if the bend is over
        if (mPortVoiceStates[i].samplesRemaining == 0)
        {
          // Adjust for any accumulated floating point errors
          // In testing, this is usually accurate to around 10 significant figures
          mPortVoiceStates[i].currentFreq = mPortVoiceStates[i].targetFreq;
          voices[i].frequency = mPortVoiceStates[i].currentFreq;

          // Turn off
          mPortVoiceStates[i].isBending = false;
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
