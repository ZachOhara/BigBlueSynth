#pragma once

#include "IGraphicsStructs.h"
#include "IControls.h"

#include "parameters.h"

#include "BigBlueAudioModule.h"
#include "BigBlueGraphics.h"
#include "VoiceManager.h"
#include "TuningProcessor.h"
#include "Oscillator.h"
#include "OscMixer.h"
#include "EnvelopeProcessor.h"
#include "FilterProcessor.h"
#include "PitchModulationProcessor.h"

const iplug::igraphics::IColor COLOR_MAT_BGRAY900(255, 38, 50, 56);

class BBInterfaceManager
{
public:
  BBInterfaceManager();
  ~BBInterfaceManager();

  void SetDelegate(IEditorDelegate* mDelegate);

  // TODO: Genericize the layout function
  virtual void LayoutFunction(IGraphics* pGraphics);

  void NotifyParamChange(int pid);
  void ProcessQueuedUpdates();

private:
  static const int QUEUE_SIZE = 128;
  IPlugQueue<int> mParamChangeQueue;

  IEditorDelegate* mDelegate = 0;

  // Pointers to specific visual elements
  IControl* mpPortamentoTypeSwitch = 0;
  IControl* mpPortamentoTimeKnob = 0;
  IControl* mpPortamentoRateKnob = 0;

  void OnParamChange(int pid);

  const IParam* GetParam(int pid);
};
