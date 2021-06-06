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
      if (voices[i].event == EVoiceEvent::kNoteStart && mCurrentMode != kPortamentoModeOff)
      {
        // Initialize the portamento
        HandleNoteStart(&voices[i], &mPortVoiceStates[i]);
      }
      else if (mPortVoiceStates[i].isBending)
      {
        // This block is only reached if the voice is on its 2nd (or later)
        // sample of portamento

        // Increment the frequecy (multipicatively)
        mPortVoiceStates[i].currentFreq *= mPortVoiceStates[i].deltaFreq;
        voices[i].frequency = mPortVoiceStates[i].currentFreq;

        // Update the timer and check if the bend is over
        mPortVoiceStates[i].samplesRemaining--;
        if (mPortVoiceStates[i].samplesRemaining == 0)
        {
          // Adjust for any accumulated floating point errors
          // In testing, this is usually still accurate to around 10 significant figures
          voices[i].frequency = mPortVoiceStates[i].targetFreq;

          // Turn off
          mPortVoiceStates[i].isBending = false;
        }
      }
    }
  }
}

void PortamentoProcessor::SetPortamentoMode(EPortamentoMode mode)
{
  mCurrentMode = mode;
}

void PortamentoProcessor::SetPortamentoTime(double seconds)
{
  mPortamentoTime = seconds;
}

void PortamentoProcessor::SetPortamentoRate(double secondsPerSemitone)
{
  mPortamentoRate = secondsPerSemitone;
}

void PortamentoProcessor::HandleNoteStart(VoiceState* voice, PortamentoVoiceState* portVoiceState)
{


  // TODO: determine this values
  double totalModulation = 12; // in semitones

  // Let's get bending
  portVoiceState->isBending = true;

  // Set the start, current, and target frequencies
  portVoiceState->startFreq = voice->frequency / GetPitchMultiplier(totalModulation);
  portVoiceState->currentFreq = portVoiceState->startFreq;
  portVoiceState->targetFreq = voice->frequency;

  // Get modulation time
  double durationTime;
  if (mCurrentMode == kPortamentoModeTime)
    durationTime = mPortamentoTime;
  else if (mCurrentMode == kPortamentoModeRate)
    durationTime = mPortamentoRate * totalModulation;
  portVoiceState->samplesRemaining = durationTime * SampleRate();

  // Calculate increment per sample
  double semitonesPerSample = totalModulation / portVoiceState->samplesRemaining;
  portVoiceState->deltaFreq = GetPitchMultiplier(semitonesPerSample);

  // Send the start frequency back to the voice
  voice->frequency = portVoiceState->currentFreq;
}
