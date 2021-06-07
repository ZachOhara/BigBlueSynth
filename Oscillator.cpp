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
      // TODO fix this sloppy hack so this calculation doesn't happen every frame
        //mOscVoiceStates[i].lastBaseFreq = voices[i].frequency;
        //mOscVoiceStates[i].modFrequency = GetModFrequency(mOscVoiceStates[i].lastBaseFreq);
        //mOscVoiceStates[i].phaseIncrement = mOscVoiceStates[i].modFrequency / SampleRate();

      //}

      // NOTE: the if statement above is commented so the osc can respond instantly to settings changes
      // Consider removing it (and lastBaseFreq) permanently
      // OR add a settings dirty flag



      // Set up new (or changing) voices
      if (mIsModDirty || voices[i].frequency != mOscVoiceStates[i].lastBaseFreq)
        // This check covers new notes, but also pitch bends, glides, etc.
        // If the oscillator settings have changed, the dirty flag will also catch that
      {
        mOscVoiceStates[i].lastBaseFreq = voices[i].frequency;
        mOscVoiceStates[i].modFrequency = GetModFrequency(mOscVoiceStates[i].lastBaseFreq);
        mOscVoiceStates[i].phaseIncrement = mOscVoiceStates[i].modFrequency / SampleRate();
      }

      // Clean up ending voices
      if (voices[i].event == EVoiceEvent::kNoteEnd)
      {
        mOscVoiceStates[i].lastBaseFreq = 0;
        mOscVoiceStates[i].modFrequency = 0;
        mOscVoiceStates[i].phasePosition = 0;
        mOscVoiceStates[i].phaseIncrement = 0;
        mOscVoiceStates[i].oscSampleValue = 0;
        continue; // don't process the sample for ending notes
      }

      // Process the sample
      mOscVoiceStates[i].oscSampleValue = GetSample(mOscVoiceStates[i].phasePosition);
      mOscVoiceStates[i].phasePosition += mOscVoiceStates[i].phaseIncrement;
      if (mOscVoiceStates[i].phasePosition > 1)
      {
        mOscVoiceStates[i].phasePosition -= 1;
      }
      else
      {
        // The phase rollover is also the ideal place to end notes,
        // so stop it from ending on this non-rollover frame

        // voices[i].isReadyToEnd = false;

        // NOTE: this may create problems with very low frequencies
        // At most the delay in ending the note here is about 1/20 seconds
        // If this turns out to be an issue we'll address it later

        // UPDATE: this creates big problems as soon as we add a second oscillator
        // since now the note can only end once the phase rollovers of both
        // oscillators coincide. I'm removing it for now and we might have to find
        // a different way to prevent abrupt chopping.
      }
    }
  }

  // At the end of the frame, the frequecy mod will have been updated
  mIsModDirty = false;
}

double Oscillator::GetSampleValue(int voiceIdx)
{
  return mOscVoiceStates[voiceIdx].oscSampleValue;
}

void Oscillator::SetWaveform(EWaveform waveform)
{
  mWaveform = waveform;
  RecalculateMods();
}

void Oscillator::SetOctaveMod(double octaveMod)
{
  mOctaveMod = octaveMod;
  RecalculateMods();
}

void Oscillator::SetSemitoneMod(double semitoneMod)
{
  mSemitoneMod = semitoneMod;
  RecalculateMods();
}

void Oscillator::SetCentsMod(double centsMod)
{
  mCentsMod = centsMod;
  RecalculateMods();
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
}

void Oscillator::RecalculateMods()
{
  static const double SINGLE_SEMITONE = std::pow(2.0, 1.0 / 12.0);
  double octaveFactor = std::pow(2, mOctaveMod);
  double semitones = mSemitoneMod + (mCentsMod / 100);
  double semitoneFactor = std::pow(SINGLE_SEMITONE, semitones);

  mCumulFreqMod = octaveFactor * semitoneFactor;
  mIsModDirty = true;
}

double Oscillator::GetModFrequency(double baseFreq)
{
  return baseFreq * mCumulFreqMod;
}

SubOscillator::SubOscillator()
{
  SetOctaveMod(-1.0);
  mLowOctaveOsc.SetOctaveMod(-2.0);
}

SubOscillator::~SubOscillator()
{
}

void SubOscillator::ProcessVoices(VoiceState* voices)
{
  Oscillator::ProcessVoices(voices);
  mLowOctaveOsc.ProcessVoices(voices);
}

double SubOscillator::GetSampleValue(int voiceIdx)
{
  double high = Oscillator::GetSampleValue(voiceIdx);
  double low = mLowOctaveOsc.GetSampleValue(voiceIdx);
  low *= mLowOctaveBlend;
  return high + low;
}

void SubOscillator::HandleReset()
{
  mLowOctaveOsc.SetSampleRate(SampleRate());
}

void SubOscillator::SetSubWaveform(ESubWaveform waveform)
{
  switch (waveform)
  {
  case kSubSquareWave:
    SetWaveform(kSquareWave);
    mLowOctaveOsc.SetWaveform(kSquareWave);
    break;
  case kSubSawtoothWave:
    SetWaveform(kSawtoothWave);
    mLowOctaveOsc.SetWaveform(kSawtoothWave);
    break;
  }
}

void SubOscillator::SetLowOctaveBlend(double blend)
{
  mLowOctaveBlend = blend;
}
