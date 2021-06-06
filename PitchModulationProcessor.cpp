#include "PitchModulationProcessor.h"


double GetPitchMultiplier(double semitones)
{
  return pow(SEMITONE, semitones);
}

double GetSemitones(double pitchMultiplier)
{
  // This is the same as a log base-semitone of the multiplier
  return log(pitchMultiplier) / log(SEMITONE);
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
  // Initialize arrays
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    mRawVoiceFrequencies[i] = 0;
    mVoiceOrderPressed[i] = 0;
  }
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
        // Track note starts, even when portamento is off
        TrackNoteStartOrder(i, voices[i].frequency);
        // If portamento is on, initialize it here
        if (mCurrentMode != kPortamentoModeOff)
        {
          double startFrequency = GetPortamentoStartFrequency(voices);
          if (startFrequency > 0)
          {
            InitializeVoicePortamento(&voices[i], &mPortVoiceStates[i], startFrequency);
          }
        }
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
      // Track note ends, regardless of portamento status
      // Accurate isBending values are necessary for GetPortamentoStartFrequency() to work
      if (voices[i].event == EVoiceEvent::kNoteEnd)
      {
        mPortVoiceStates[i].isBending = false;
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

void PortamentoProcessor::InitializeVoicePortamento(VoiceState* voice, PortamentoVoiceState* portVoiceState, double startFrequency)
{
  // Determine the total semitone modulation from the frequency ratio
  double totalModulation = GetSemitones(voice->frequency / startFrequency);
  // Set the start, current, and target frequencies
  portVoiceState->startFreq = voice->frequency / GetPitchMultiplier(totalModulation);
  portVoiceState->currentFreq = portVoiceState->startFreq;
  portVoiceState->targetFreq = voice->frequency;
  // Get modulation time
  double durationTime = 0;
  if (mCurrentMode == kPortamentoModeTime)
    durationTime = mPortamentoTime;
  else if (mCurrentMode == kPortamentoModeRate)
    durationTime = mPortamentoRate * abs(totalModulation);
  portVoiceState->samplesRemaining = durationTime * SampleRate();
  // Make sure the time is actually non-zero
  if (portVoiceState->samplesRemaining > 0)
  {
    // Calculate increment per sample
    double semitonesPerSample = totalModulation / portVoiceState->samplesRemaining;
    portVoiceState->deltaFreq = GetPitchMultiplier(semitonesPerSample);
    // Send the start frequency back to the voice
    voice->frequency = portVoiceState->currentFreq;
    // Let's get bending
    portVoiceState->isBending = true;
  }
  else
  {
    // If the duration is actually zero, don't do any portamento
    portVoiceState->isBending = false;
  }
}

void PortamentoProcessor::TrackNoteStartOrder(int voiceIdx, double frequency)
{
  // Find the current position of the new voice
  int currentVoiceIndex = mVoiceOrderPressed[voiceIdx];
  // Increment everything currently ahead of it
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (mVoiceOrderPressed[i] <= currentVoiceIndex)
    {
      mVoiceOrderPressed[i]++;
    }
  }
  // Assign the new voice to position 0 and store the frequency
  mVoiceOrderPressed[voiceIdx] = 0;
  mRawVoiceFrequencies[voiceIdx] = frequency;
}

double PortamentoProcessor::GetPortamentoStartFrequency(VoiceState* voices)
{
  // Search currently sounding voices for the most recently attacked
  int lastVoiceIdx = -1;
  int minPressOrder = MAX_NUM_VOICES;
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (voices[i].isSounding && mVoiceOrderPressed[i] < minPressOrder)
    {
      // Exclude voice order 0
      // That is the note we are trying to find a start frequency for
      if (mVoiceOrderPressed[i] != 0)
      {
        lastVoiceIdx = i;
        minPressOrder = mVoiceOrderPressed[i];
      }
    }
  }
  // If no voices are currently sounding, find the last attacked anyway
  if (lastVoiceIdx == -1)
  {
    for (int i = 0; i < MAX_NUM_VOICES; i++)
    {
      // Use 1 as the most recent instead of 0, for the same reason as above
      // Voice order 0 is the note we are trying to find a start frequency for
      if (mVoiceOrderPressed[i] == 1)
      {
        lastVoiceIdx = i;
      }
    }
  }

  // If nothing is found, it means this is the first note. Return -1 as an error.
  if (lastVoiceIdx == -1)
  {
    return 0;
  }

  // If the most recent voice is also currently in portamento, return its current frequency
  if (mPortVoiceStates[lastVoiceIdx].isBending)
  {
    return mPortVoiceStates[lastVoiceIdx].currentFreq;
  }
  // Otherwise, return the initial unmodified frequency
  return mRawVoiceFrequencies[lastVoiceIdx];
}
