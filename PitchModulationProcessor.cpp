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
      // Track note starts, even when portamento is off
      if (voices[i].event == EVoiceEvent::kNoteStart)
      {
        // Get the start frequency first, since TrackNoteStartOrder() will screw it up
        double startFrequency = GetPortamentoStartFrequency(voices);
        TrackNoteStartOrder(i, voices[i].frequency);
        // If portamento is on, initialize it here
        if (mCurrentMode != kPortamentoModeNever && startFrequency > 0)
        {
          InitializeVoicePortamento(&voices[i], &mPortVoiceStates[i], startFrequency);
        }
        // If portamento is off, make sure we're not still bending
        else
        {
          mPortVoiceStates[i].isBending = false;
        }
      }
      // This is reached on the 2nd (or later) sample of portamento
      else if (mPortVoiceStates[i].isBending)
      {
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
    // If a note is not sounding but the portamento hasn't finished, continue silently
    // This is necessary for starting new notes, except in legato mode
    else if (mPortVoiceStates[i].isBending)
    {
      mPortVoiceStates[i].currentFreq *= mPortVoiceStates[i].deltaFreq;
      mPortVoiceStates[i].samplesRemaining--;
      if (mPortVoiceStates[i].samplesRemaining == 0)
        mPortVoiceStates[i].isBending = false;
    }
  }
}

void PortamentoProcessor::SetPortamentoMode(EPortamentoMode mode)
{
  mCurrentMode = mode;
}

void PortamentoProcessor::SetPortamentoType(EPortamentoType type)
{
  mCurrentType = type;
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
  if (mCurrentType == kPortamentoTypeTime)
    durationTime = mPortamentoTime;
  else if (mCurrentType == kPortamentoTypeRate)
    durationTime = mPortamentoRate * abs(totalModulation);
  portVoiceState->samplesRemaining = durationTime * SampleRate();
  // Make sure the duration is actually non-zero
  // If duration is zero, do nothing
  if (portVoiceState->samplesRemaining > 0)
  {
    // Calculate increment per sample
    double semitonesPerSample = totalModulation / portVoiceState->samplesRemaining;
    portVoiceState->deltaFreq = GetPitchMultiplier(semitonesPerSample);
    // Send the start frequency back to the voice
    voice->frequency = portVoiceState->currentFreq;
    // Let's get bending!
    portVoiceState->isBending = true;
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
  // Note: this method is called before the note order is updated,
  // meaning that position 0 will not be the note currently being attacked
  int lastVoiceIdx = -1;
  int minPressOrder = MAX_NUM_VOICES;
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (voices[i].isSounding && voices[i].event != kNoteStart
      && mVoiceOrderPressed[i] < minPressOrder)
    {
      lastVoiceIdx = i;
      minPressOrder = mVoiceOrderPressed[i];
    }
  }
  // If no voices are currently sounding and we are NOT in legato mode, find the last attacked anyway
  if (lastVoiceIdx == -1 && mCurrentMode != kPortamentoModeLegato)
  {
    for (int i = 0; i < MAX_NUM_VOICES; i++)
    {
      if (mVoiceOrderPressed[i] == 0)
      {
        lastVoiceIdx = i;
      }
    }
  }
  // If nothing is found, it means this is the first note. Return a null result.
  if (lastVoiceIdx == -1)
  {
    return 0;
  }
  // Result is found, now get the frequency
  // If the most recent voice is currently in portamento, return its current frequency
  if (mPortVoiceStates[lastVoiceIdx].isBending)
  {
    return mPortVoiceStates[lastVoiceIdx].currentFreq;
  }
  // Otherwise, return the initial unmodified frequency
  return mRawVoiceFrequencies[lastVoiceIdx];
}

VibratoProcessor::VibratoProcessor()
{
}

VibratoProcessor::~VibratoProcessor()
{
}

void VibratoProcessor::ProcessVoices(VoiceState* voices)
{
  double currentMod = mVibratoDepth * sin(2 * M_PI * mPhasePosition);
  double pitchMultiplier = GetPitchMultiplier(currentMod);
  bool isAnythingSounding = false;
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (voices[i].isSounding)
    {
      isAnythingSounding = true;
      if (voices[i].frequency != mVibratoVoiceStates[i].lastFreq)
      {
        mVibratoVoiceStates[i].initialFreq = voices[i].frequency;
      }

      voices[i].frequency = mVibratoVoiceStates[i].initialFreq * pitchMultiplier;
      mVibratoVoiceStates[i].lastFreq = voices[i].frequency;
    }
  }
  if (isAnythingSounding)
  {
    mPhasePosition += mPhaseIncrement;
  }
  else
  {
    mPhasePosition = 0;
  }
}

void VibratoProcessor::SetVibratoRate(double hz)
{
  mPhaseIncrement = hz / SampleRate();
}

void VibratoProcessor::SetVibratoDepth(double semitones)
{
  mVibratoDepth = semitones;
}
