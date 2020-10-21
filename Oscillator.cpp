#include "Oscillator.h"

Oscillator::Oscillator()
{
}

Oscillator::~Oscillator()
{
}

void Oscillator::ProcessVoices(VoiceState* voices)
{
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (voices[i].isSounding)
    {
      // Set up new (or changing) voices
      //if (voices[i].frequency != mOscVoiceStates[i].lastBaseFreq)
        // This check covers new notes, but also pitch bends, glides, etc.
      //{
        mOscVoiceStates[i].lastBaseFreq = voices[i].frequency;
        mOscVoiceStates[i].modFrequency = GetModFrequency(mOscVoiceStates[i].lastBaseFreq);
        mOscVoiceStates[i].phaseIncrement = mOscVoiceStates[i].modFrequency / 44000; // TODO replace this hack

      //}

      // NOTE: the if statement above is commented so the osc can respond instantly to settings changes
      // Consider removing it (and lastBaseFreq) permanently  

      // Clean up ending voices
      if (voices[i].event == EVoiceEvent::kNoteEnd)
      {
        mOscVoiceStates[i].lastBaseFreq = 0;
        mOscVoiceStates[i].modFrequency = 0;
        mOscVoiceStates[i].phasePosition = 0;
        mOscVoiceStates[i].phaseIncrement = 0;
        voices[i].sampleValue = 0;
        continue; // don't process the sample for ending notes
      }

      // Process the sample
      voices[i].sampleValue = GetSample(mOscVoiceStates[i].phasePosition);
      mOscVoiceStates[i].phasePosition += mOscVoiceStates[i].phaseIncrement;
      if (mOscVoiceStates[i].phasePosition > 1)
      {
        mOscVoiceStates[i].phasePosition -= 1;
      }
      else
      {
        // The phase rollover is also the ideal place to end notes,
        // so stop it from ending on this non-rollover frame
        voices[i].isReadyToEnd = false;
        // NOTE: this may create problems with very low frequencies
        // At most the delay in ending the note here is about 1/20 seconds
        // If this turns out to be an issue we'll address it later
      }
    }
  }
}

void Oscillator::SetOctaveMod(int octaveMod)
{
  mOctaveMod = octaveMod;
}

void Oscillator::SetWaveform(EWaveform waveform)
{
  mWaveform = waveform;
}

double Oscillator::GetSample(double phasePos)
{
  // Calculate the current sample value
  switch (mWaveform) {
  case kSineWave:
    return sin(2 * M_PI * phasePos);
  case kTriangleWave:
    if (phasePos < 0.5)
      return 1 - (4 * abs(phasePos - 0.25));
    else
      return (4 * abs(phasePos - 0.75)) - 1;
  case kSquareWave:
    return (phasePos < 0.5 ? 1.0 : -1.0);
  case kSawtoothWave:
    return 1 - (2 * phasePos);
  default:
    return 0;
  }

  /*
  // Update the phase for the next cycle
  mPhasePosition += mPhaseIncrement;
  if (mPhasePosition > 1)
    mPhasePosition -= 1;

  return sampleValue;
  */
}

/*
void Oscillator::HandleParamChange(int paramType, double newValue, int newIntValue)
{
  switch (paramType) {
  case kFrequencyParam:
    SetFrequency(newValue);
    break;
  case kWaveformParam:
    SetWaveform(newIntValue);
    break;
  }
}

*/

double Oscillator::GetModFrequency(double baseFreq)
{
  double octaveFactor = std::pow(2, mOctaveMod);
  baseFreq *= octaveFactor;
  return baseFreq;
}
