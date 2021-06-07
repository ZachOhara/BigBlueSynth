#pragma once

#include "IPlug_include_in_plug_hdr.h"

#include "IGraphicsStructs.h"
#include "IControls.h"

#include "parameters.h"

#include "BBInterfaceManager.h"
#include "BigBlueAudioModule.h"
#include "VoiceManager.h"
#include "TuningProcessor.h"
#include "Oscillator.h"
#include "OscMixer.h"
#include "EnvelopeProcessor.h"
#include "FilterProcessor.h"
#include "PitchModulationProcessor.h"

#include <vector>

const int kNumPresets = 1;

using namespace iplug;
using namespace igraphics;

class BigBluePlugin : public Plugin
{
public:
  BigBluePlugin(const InstanceInfo& info, int nParams, int nPresets);

  void RegisterModule(BigBlueAudioModule* module);

  void OnReset() override;

private:
  std::vector<BigBlueAudioModule*> mModules;
};

class BigBlueTest final : public BigBluePlugin
{
public:
  BigBlueTest(const InstanceInfo& info);

  void OnIdle() override;
  void OnParamChange(int pid) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;

private:
  IMidiQueue mSysMidiQueue; // This contains only system messages, not note information

  BBInterfaceManager mBBInterfaceManager;

  VoiceManager mVoiceManager;
  TuningProcessor mTuningProc;
  PortamentoProcessor mPortamentoProcessor;
  Oscillator mOscillator1;
  Oscillator mOscillator2;
  OscMixer mOscMixer;
  EnvelopeProcessor mEnvelopeProcessor;
  LowPassFilterProcessor mFilterProcessor;
  PitchWheelProcessor mPitchWheelProcessor;

  void ProcessSystemMessages(int sampleOffset);
};
