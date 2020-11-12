#pragma once

#include "IPlug_include_in_plug_hdr.h"

class BigBlueAudioModule
{
public:
  BigBlueAudioModule();

  void SetSampleRate(double sampleRate);

  virtual void HandleReset() {};

protected:
  double SampleRate();

private:
  double mSampleRate = 0.0;
};

