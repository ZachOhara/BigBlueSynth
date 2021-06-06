#include "EnvelopeProcessor.h"

EnvelopeProcessor::EnvelopeProcessor()
{
}

EnvelopeProcessor::~EnvelopeProcessor()
{
}

void EnvelopeProcessor::ProcessVoices(VoiceState* voices)
{
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (voices[i].isSounding)
    {
      // Define for convenience
      EnvVoiceState* envVoice = &mEnvVoiceStates[i];
      // First handle note attaks / releases
      if (voices[i].event == kNoteStart)
      {
        ProgressToSegment(envVoice, kAttackSegment, mAttackTime, mPeakLevel);
      }
      else if (voices[i].event == kNoteRelease)
      {
        ProgressToSegment(envVoice, kReleaseSegment, mReleaseTime, 0.0);
      }
      // Keep alive voices that are still releasing
      if (voices[i].nFramesSinceRelease >= 0)
      {
        if (envVoice->currentSegment == kReleaseSegment)
        {
          voices[i].isReadyToEnd = false;
        }
      }
      // Now do the math parts
      // (skip voices that are not changing volume)
      if (!IsInStationarySegment(envVoice)) {
        double progress = 1.0 - (((double)envVoice->remainingSamples) / ((double)envVoice->segmentSamples));
        double scaledProgress = progress; // Exponent version: = pow(progress, mSegmentExponent);
        double currentDelta = scaledProgress * envVoice->segmentDifference;
        envVoice->currentOutput = currentDelta + envVoice->segmentInitialOutput;
        envVoice->remainingSamples--;
        // Apply the change to the voice:
        // The ^(3/2) exponent is related to human amplitude / loudness perception
        voices[i].sampleValue *= pow(envVoice->currentOutput, 3.0 / 2.0);
        // Progress the segment
        while (envVoice->remainingSamples <= 0 && !IsInStationarySegment(envVoice)) {
          switch (envVoice->currentSegment) {
            // No case is necessary for silence or sustains, since these will
            // Be progressed by external events (note attacks and releases)
          case kAttackSegment:
            ProgressToSegment(envVoice, kDecaySegment, mDecayTime, mSustainLevel);
            break;
          case kDecaySegment:
            ProgressToSegment(envVoice, kSustainSegment, 0.0, 0.0);
            break;
          case kReleaseSegment:
            ProgressToSegment(envVoice, kSilenceSegment, 0.0, 0.0);
            break;
          }
        }
      }
      // Stationary segments still need processing
      // TODO restructure this to eliminate this calculation every frame
      else
      {
        voices[i].sampleValue *= pow(envVoice->currentOutput, 3.0 / 2.0);
      }
    }
  }
}

void EnvelopeProcessor::SetAttackTime(double millis)
{
  mAttackTime = millis / 1000.0;
}

void EnvelopeProcessor::SetDecayTime(double millis)
{
  mDecayTime = millis / 1000.0;
}

void EnvelopeProcessor::SetSustainLevel(double loudness)
{
  mSustainLevel = loudness;
}

void EnvelopeProcessor::SetReleaseTime(double millis)
{
  mReleaseTime = millis / 1000.0;
}

void EnvelopeProcessor::SetPeakLevel(double loudness)
{
  mPeakLevel = loudness;
}

bool EnvelopeProcessor::IsInStationarySegment(EnvVoiceState* voice)
{
  return voice->currentSegment == kSilenceSegment || voice->currentSegment == kSustainSegment;
}

void EnvelopeProcessor::ProgressToSegment(EnvVoiceState* voice, int newSegment, double duration, double goal)
{
  // TODO: when exponent support is added, 'exponent' should be a parameter heremCurrentSegment = newSegment;
  voice->currentSegment = newSegment;
  double sps = SecondsPerSample();
  voice->segmentSamples = (int)std::floor(duration / SecondsPerSample());
  voice->remainingSamples = voice->segmentSamples;
  // mSegmentExponent = exponent;
  if (IsInStationarySegment(voice)) {
    voice->segmentInitialOutput = 0;
    voice->segmentDifference = 0;
    if (voice->currentSegment == kSilenceSegment)
      voice->currentOutput = 0.0;
    else // in sustain segment
      voice->currentOutput = mSustainLevel;
  }
  else {
    voice->segmentInitialOutput = voice->currentOutput;
    voice->segmentDifference = goal - voice->currentOutput;
  }
}
