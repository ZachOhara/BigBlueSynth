#pragma once

#include "IPlug_include_in_plug_hdr.h"

#include "IGraphicsStructs.h"
#include "IControls.h"

#include "BigBlueAudioModule.h"
#include "BigBlueGraphics.h"
#include "VoiceManager.h"
#include "TuningProcessor.h"
#include "Oscillator.h"
#include "OscMixer.h"
#include "EnvelopeProcessor.h"
#include "FilterProcessor.h"
#include "PitchModulationProcessor.h"

#include <vector>

const int kNumPresets = 1;

const iplug::igraphics::IColor COLOR_MAT_BGRAY900(255, 38, 50, 56);

enum EParams
{
  // General settings
  kActiveVoices,
  // Portamento
  kPortamentoMode,
  kPortamentoTime,
  kPortamentoRate,
  // Oscillators
  kOsc1OctavePid,
  kOsc1WaveformPid,
  kOsc1SemitonePid,
  kOsc1DetunePid,
  kOsc2OctavePid,
  kOsc2WaveformPid,
  kOsc2SemitonePid,
  kOsc2DetunePid,
  // Osc mixer
  kMixLevelOsc1,
  kMixLevelOsc2,
  // Envelope
  kEnvAttackPid,
  kEnvDecayPid,
  kEnvSustainPid,
  kEnvReleasePid,
  kEnvPeakPid,
  // Filter
  kFilCutoffPid,
  kNumParams
};

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

  // These pointers are so the knobs can be shown/hide/disabled
  IControl* mpPortamentoTimeKnob;
  IControl* mpPortamentoRateKnob;

  VoiceManager mVoiceManager;
  TuningProcessor mTuningProc;
  PortamentoProcessor mPortamentoProcessor;
  Oscillator mOscillator1;
  Oscillator mOscillator2;
  OscMixer mOscMixer;
  EnvelopeProcessor mEnvelopeProcessor;
  FilterProcessor mFilterProcessor;
  PitchWheelProcessor mPitchWheelProcessor;

  void ProcessSystemMessages(int sampleOffset);

  // Thread data exchange system
  static const int GRAPHICS_FUNCTION_QUEUE_SIZE = 64;
  IPlugQueue<std::function<void()>*> mGraphicsFunctionQueue;
  void QueueGraphicsFunction(std::function<void()>* function);
  void DoQueuedGraphicsFunctions();
};
