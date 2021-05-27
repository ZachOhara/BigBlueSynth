#include "FilterProcessor.h"

FilterProcessor::FilterProcessor()
{
  //RecalculateFilterKernel();
}

FilterProcessor::~FilterProcessor()
{
}

void FilterProcessor::ProcessVoices(VoiceState* voices)
{
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    // Place the current sample value
    int place = mFilterVoiceStates[i].placeIndex;
    mFilterVoiceStates[i].lastSamples[place] = voices[i].sampleValue;
    mFilterVoiceStates[i].placeIndex++;
    if (mFilterVoiceStates[i].placeIndex == KERNEL_SIZE)
    {
      mFilterVoiceStates[i].placeIndex = 0;
    }
    // Calculate the convolution
    double result = 0;
    for (int j = KERNEL_SIZE - 1; j >= 0; j--)
    {
      result += mFilterVoiceStates[i].lastSamples[place] * mFilterKernel[j];
      place--;
      if (place < 0)
      {
        place = KERNEL_SIZE - 1;
      }
    }
    // Swap the result
    voices[i].sampleValue = result;
  }
}

void FilterProcessor::SetCutoffFrequency(double hz)
{
  mCutoffFrequency = hz;
  RecalculateFilterKernel();
}

void FilterProcessor::SetResonance(double resonance)
{
  mResonance = resonance;
}

void FilterProcessor::RecalculateFilterKernel()
{
  int sampleAdjust = (KERNEL_SIZE / 2) - 1;
  double cutoffRatio;
  if (SampleRate() > 0)
  {
    cutoffRatio = mCutoffFrequency / SampleRate();
  }
  else
  {
    cutoffRatio = 0.5;
  }

  for (int i = 0; i < KERNEL_SIZE; i++)
  {
    int relativeSample = i - sampleAdjust;
    if (relativeSample != 0)
      mFilterKernel[i] = sin(2 * PI * cutoffRatio * relativeSample) / (relativeSample * PI);
    else
      mFilterKernel[i] = 2 * cutoffRatio;
  }
  ApplyKernelWindow();
}

void FilterProcessor::ApplyKernelWindow()
{
  // Blackman window
  for (int i = 0; i < KERNEL_SIZE; i++)
  {
    double position = ((double)i) / KERNEL_SIZE;
    double value = 0.54 - (0.46 * cos(2 * PI * position));
    mFilterKernel[i] *= value;
  }

}

void FilterProcessor::FFT(double* in, double* out)
{
}

void FilterProcessor::IFFT(double* in, double* out)
{
}
