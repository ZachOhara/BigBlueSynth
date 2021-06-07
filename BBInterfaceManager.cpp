#include "BBInterfaceManager.h"

BBInterfaceManager::BBInterfaceManager() :
  mParamChangeQueue(QUEUE_SIZE)
{
}

BBInterfaceManager::~BBInterfaceManager()
{
}

void BBInterfaceManager::SetDelegate(IEditorDelegate* newDelegate)
{
  mDelegate = newDelegate;
}

void BBInterfaceManager::LayoutFunction(IGraphics* pGraphics)
{
  // General setup
  pGraphics->AttachPanelBackground(COLOR_MAT_BGRAY900);
  pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
  const IRECT window = pGraphics->GetBounds();
  // Oscillator 1
  const IRECT osc1Box = IRECT(20, 35, 245, 115);
  pGraphics->AttachControl(new ITextControl(osc1Box.GetVShifted(-30).GetFromTop(30), "Oscillator 1", IText(17, COLOR_WHITE)));
  pGraphics->AttachControl(new BBSlideSelectControl(pGraphics, osc1Box.GetHShifted(0).GetHSliced(40), kOsc1OctavePid, OCTAVE_NAMES, "Octave", true));
  pGraphics->AttachControl(new BBSlideSelectControl(pGraphics, osc1Box.GetHShifted(50).GetHSliced(65), kOsc1WaveformPid, WAVEFORM_NAMES, "Waveform", false));
  pGraphics->AttachControl(new BBKnobControl(osc1Box.GetHShifted(110).GetHSliced(65).GetCentredInside(65), kOsc1SemitonePid, "Semitone", BB_DEFAULT_ACCENT_COLOR, -135.f, 135.f, 0.f));
  pGraphics->AttachControl(new BBKnobControl(osc1Box.GetHShifted(160).GetHSliced(65).GetCentredInside(65), kOsc1DetunePid, "Detune", BB_DEFAULT_ACCENT_COLOR, -135.f, 135.f, 0.f));
  // Oscillator 2
  const IRECT osc2Box = osc1Box.GetVShifted(120);
  pGraphics->AttachControl(new ITextControl(osc2Box.GetVShifted(-30).GetFromTop(30), "Oscillator 2", IText(17, COLOR_WHITE)));
  pGraphics->AttachControl(new BBSlideSelectControl(pGraphics, osc2Box.GetHShifted(0).GetHSliced(40), kOsc2OctavePid, OCTAVE_NAMES, "Octave", true));
  pGraphics->AttachControl(new BBSlideSelectControl(pGraphics, osc2Box.GetHShifted(50).GetHSliced(65), kOsc2WaveformPid, WAVEFORM_NAMES, "Waveform", false));
  pGraphics->AttachControl(new BBKnobControl(osc2Box.GetHShifted(110).GetHSliced(65).GetCentredInside(65), kOsc2SemitonePid, "Semitone", BB_DEFAULT_ACCENT_COLOR, -135.f, 135.f, 0.f));
  pGraphics->AttachControl(new BBKnobControl(osc2Box.GetHShifted(160).GetHSliced(65).GetCentredInside(65), kOsc2DetunePid, "Detune", BB_DEFAULT_ACCENT_COLOR, -135.f, 135.f, 0.f));
  // Mixer
  const IRECT mixerBox = osc2Box.GetVShifted(120);
  pGraphics->AttachControl(new ITextControl(mixerBox.GetVShifted(-30).GetFromTop(30), "Osc Mixer", IText(17, COLOR_WHITE)));
  pGraphics->AttachControl(new BBSliderControl(mixerBox.GetHShifted(0).GetHSliced(40), kMixLevelOsc1, "Osc 1"));
  pGraphics->AttachControl(new BBSliderControl(mixerBox.GetHShifted(40).GetHSliced(40), kMixLevelOsc2, "Osc 2"));
  // Envelope
  IRECT envelopeBox = osc1Box.GetHShifted(240);
  envelopeBox.Alter(0, 0, 0, 40);
  double h = 25;
  pGraphics->AttachControl(new ITextControl(envelopeBox.GetVShifted(-30).GetFromTop(30), "Envelope", IText(17, COLOR_WHITE)));
  pGraphics->AttachControl(new BBSliderControl(envelopeBox.GetHShifted(h + 0).GetHSliced(40), kEnvAttackPid, "Attack", BB_DEFAULT_ACCENT_COLOR, 9.f, 4.f));
  pGraphics->AttachControl(new BBSliderControl(envelopeBox.GetHShifted(h + 45).GetHSliced(40), kEnvDecayPid, "Decay", BB_DEFAULT_ACCENT_COLOR, 9.f, 4.f));
  pGraphics->AttachControl(new BBSliderControl(envelopeBox.GetHShifted(h + 90).GetHSliced(40), kEnvSustainPid, "Sustain", BB_DEFAULT_ACCENT_COLOR, 9.f, 4.f));
  pGraphics->AttachControl(new BBSliderControl(envelopeBox.GetHShifted(h + 135).GetHSliced(40), kEnvReleasePid, "Release", BB_DEFAULT_ACCENT_COLOR, 9.f, 4.f));
  // Filter
  IRECT filterBox = envelopeBox.GetVShifted(180);
  pGraphics->AttachControl(new ITextControl(filterBox.GetVShifted(-30).GetFromTop(30), "Filter", IText(17, COLOR_WHITE)));
  pGraphics->AttachControl(new BBKnobControl(filterBox.GetHShifted(20).GetHSliced(65).GetCentredInside(80), kFilCutoffPid, "Cutoff", BB_DEFAULT_ACCENT_COLOR));
  // Synth mode
  IRECT modeBox = IRECT(10, 520, 110, 590);
  pGraphics->AttachControl(new ITextControl(modeBox.GetVShifted(-35).GetFromTop(30).GetHPadded(50).GetHShifted(18), "Polyphony", IText(17, COLOR_WHITE)));
  pGraphics->AttachControl(new BBSlideSelectControl(pGraphics, modeBox.GetHSliced(55).GetFromTop(55).GetHShifted(10), kSynthMode, SYNTH_MODE_NAMES, "Mode", false));
  mpSynthVoicesKnob = new BBKnobControl(modeBox.GetCentredInside(70).GetHShifted(45), kActiveVoices, "Voices", BB_DEFAULT_ACCENT_COLOR);
  pGraphics->AttachControl(mpSynthVoicesKnob);
  // Portamento
  double portH = 160;
  IRECT portBox = IRECT(portH, 520, portH + 100, 590);
  pGraphics->AttachControl(new ITextControl(portBox.GetVShifted(-35).GetFromTop(30).GetHPadded(50).GetHShifted(33), "Portamento / Glide", IText(17, COLOR_WHITE)));
  pGraphics->AttachControl(new BBSlideSelectControl(pGraphics, portBox.GetHSliced(40), kPortamentoMode, PORTAMENTO_MODE_NAMES, "Glide", false));
  mpPortamentoTypeSwitch = new BBSlideSelectControl(pGraphics, portBox.GetHSliced(55).GetFromTop(55).GetHShifted(60), kPortamentoType, PORTAMENTO_TYPE_NAMES, "Constant", false);
  pGraphics->AttachControl(mpPortamentoTypeSwitch);
  mpPortamentoTimeKnob = new BBKnobControl(portBox.GetCentredInside(70).GetHShifted(90), kPortamentoTime, "Time", BB_DEFAULT_ACCENT_COLOR);
  mpPortamentoRateKnob = new BBKnobControl(portBox.GetCentredInside(70).GetHShifted(90), kPortamentoRate, "Rate", BB_DEFAULT_ACCENT_COLOR);
  pGraphics->AttachControl(mpPortamentoTimeKnob);
  pGraphics->AttachControl(mpPortamentoRateKnob);
  // Hide and disable both knobs for now
  // They will be unhidden depending on the portamento type
  mpPortamentoTimeKnob->SetDisabled(true);
  mpPortamentoRateKnob->SetDisabled(true);
  mpPortamentoTimeKnob->Hide(true);
  mpPortamentoRateKnob->Hide(true);
  // Vibrato
  double vibH = 300;
  IRECT vibratoBox = IRECT(vibH, 520, vibH + 100, 590);
  pGraphics->AttachControl(new ITextControl(vibratoBox.GetVShifted(-35).GetFromTop(30).GetHPadded(50).GetHShifted(44), "Vibrato", IText(17, COLOR_WHITE)));
  pGraphics->AttachControl(new ITextControl(vibratoBox.GetVShifted(-35).GetFromTop(30).GetHPadded(50).GetHShifted(44), "Vibrato", IText(17, COLOR_WHITE)));
  pGraphics->AttachControl(new BBKnobControl(vibratoBox.GetCentredInside(65).GetHShifted(20), kVibratoRatePid, "Rate", BB_DEFAULT_ACCENT_COLOR));
  pGraphics->AttachControl(new BBKnobControl(vibratoBox.GetCentredInside(65).GetHShifted(70), kVibratoDepthPid, "Depth", BB_DEFAULT_ACCENT_COLOR));

}

void BBInterfaceManager::NotifyParamChange(int pid)
{
  mParamChangeQueue.Push(pid);
}

void BBInterfaceManager::ProcessQueuedUpdates()
{
  while (mParamChangeQueue.ElementsAvailable() > 0)
  {
    int pid = 0;
    mParamChangeQueue.Pop(pid);
    OnParamChange(pid);
  }
}

void BBInterfaceManager::OnParamChange(int pid)
{
  switch (pid)
  {
    // Synth mode
    // ---------------------
  case kSynthMode:
    if (GetParam(pid)->Int() == kSynthModePoly)
    {
      mpSynthVoicesKnob->SetDisabled(false);
    }
    else
    {
      mpSynthVoicesKnob->SetDisabled(true);
    }
    break;
    // Portamento
    // ---------------------
  case kPortamentoMode:
    if (GetParam(pid)->Int() == kPortamentoModeNever)
    {
      mpPortamentoTypeSwitch->SetDisabled(true);
      mpPortamentoTimeKnob->SetDisabled(true);
      mpPortamentoRateKnob->SetDisabled(true);
    }
    else
    {
      mpPortamentoTypeSwitch->SetDisabled(false);
      mpPortamentoTimeKnob->SetDisabled(false);
      mpPortamentoRateKnob->SetDisabled(false);
    }
    break;
  case kPortamentoType:
    switch (GetParam(pid)->Int())
    {
    case kPortamentoTypeRate:
      mpPortamentoTimeKnob->Hide(true);
      mpPortamentoRateKnob->Hide(false);
      break;
    case kPortamentoTypeTime:
      mpPortamentoTimeKnob->Hide(false);
      mpPortamentoRateKnob->Hide(true);
      break;
    }
    break;

  }
}

const IParam* BBInterfaceManager::GetParam(int pid)
{
  return mDelegate->GetParam(pid);
}
