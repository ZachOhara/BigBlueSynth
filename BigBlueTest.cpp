#include "BigBlueTest.h"
#include "IPlug_include_in_plug_src.h"

BigBluePlugin::BigBluePlugin(const InstanceInfo& info, int nParams, int nPresets)
  : Plugin(info, MakeConfig(nParams, nPresets))
{
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
  };
}

void BigBluePlugin::RegisterModule(BigBlueAudioModule* module)
{
  mModules.push_back(module);
}

void BigBluePlugin::OnReset()
{
  double newSampleRate = GetSampleRate();
  for (int i = 0; i < mModules.size(); i++)
  {
    mModules[i]->SetSampleRate(newSampleRate);
    mModules[i]->HandleReset();
  }
}

void SignedDisplayFunc(double value, WDL_String& display)
{
  // Round to int
  int rounded = std::round(value);
  std::string prefix = rounded >= 0 ? "+" : "";
  std::string number = std::to_string(rounded);
  std::string compound = prefix + number;
  display.Set(compound.c_str());
}

BigBlueTest::BigBlueTest(const InstanceInfo& info)
  : BigBluePlugin(info, kNumParams, kNumPresets)
{
  // Init modules
  RegisterModule(&mTuningProc);
  RegisterModule(&mOscillator);
  // Init parameters
  GetParam(kOsc1OctavePid)->InitInt("Osc 1 Octave", 0, -1, 2, "", IParam::kFlagSignDisplay);
  GetParam(kOsc1OctavePid)->SetDisplayText(0, "+0");
  GetParam(kOsc1WaveformPid)->InitEnum("Osc 1 Waveform", EWaveform::kSineWave, WAVEFORM_NAMES);
  GetParam(kOsc1SemitonePid)->InitInt("Osc 1 Semitone", 0, -12, 12, "st", IParam::kFlagSignDisplay);
  //GetParam(kOsc1SemitonePid)->SetDisplayText(0, "+0 st");
  GetParam(kOsc1SemitonePid)->SetDisplayFunc(&SignedDisplayFunc);
  GetParam(kOsc1DetunePid)->InitDouble("Osc 1 Detune", 0, -100, 100, 0.01, "c", IParam::kFlagSignDisplay);
  GetParam(kOsc1DetunePid)->SetDisplayText(0, "+0.00 c");

  // Init interface
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachPanelBackground(COLOR_MAT_BGRAY900);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello iPlug 2!", IText(20, COLOR_WHITE)));
    pGraphics->AttachControl(new BBKnobControl(b.GetCentredInside(100).GetVShifted(-100), kOsc1WaveformPid, "Waveform"));
    auto box = b.GetCentredInside(60, 80).GetVShifted(100);
    pGraphics->AttachControl(new BBSlideSelectControl(pGraphics, box.GetCentredInside(40, 80), kOsc1OctavePid, OCTAVE_NAMES, "Octave", true));
    pGraphics->AttachControl(new BBSlideSelectControl(pGraphics, box.GetHShifted(55), kOsc1WaveformPid, WAVEFORM_NAMES, "Waveform", false));

    pGraphics->AttachControl(new BBKnobControl(box.GetHShifted(115).GetCentredInside(70).GetVShifted(0), kOsc1SemitonePid, "Semitone", BB_DEFAULT_ACCENT_COLOR, -135.f, 135.f, 0.f));
    pGraphics->AttachControl(new BBKnobControl(box.GetHShifted(165).GetCentredInside(70).GetVShifted(0), kOsc1DetunePid, "Detune", BB_DEFAULT_ACCENT_COLOR, -135.f, 135.f, 0.f));
  };
}

void BigBlueTest::OnParamChange(int pid)
{
  switch (pid)
  {
  case kOsc1OctavePid:
    mOscillator.SetOctaveMod(GetParam(pid)->Value());
    break;
  case kOsc1WaveformPid:
    mOscillator.SetWaveform((EWaveform)GetParam(pid)->Int());
    break;
  case kOsc1SemitonePid:
    mOscillator.SetSemitoneMod(GetParam(pid)->Value());
    break;
  case kOsc1DetunePid:
    mOscillator.SetCentsMod(GetParam(pid)->Value());
    break;
  default:
    break;
  }
}

void BigBlueTest::ProcessMidiMsg(const IMidiMsg& msg)
{

  //if (msg.StatusMsg() == IMidiMsg::kNoteOn) {
  //  mOscillator.SetFrequency(mTuningProc.GetFrequency(msg.NoteNumber()));
  //}

  mVoiceManager.ProcessMidiMessage(msg);
}

void BigBlueTest::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = 0.7; // GetParam(kOsc1WaveformPid)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {

    
    VoiceState* voices = mVoiceManager.AdvanceFrame();

    // Process each voice through the modules
    mTuningProc.ProcessVoices(voices);
    mOscillator.ProcessVoices(voices);

    // Combine the voices into a sample value
    double sample = 0;
    for (int i = 0; i < MAX_NUM_VOICES; i++) {
      sample += voices[i].sampleValue;
    }

    // This is an adequate way to prevent clipping
    sample /= MAX_NUM_VOICES;

    // Assign the signal to each channel
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = sample * gain;
    }
  }

  mVoiceManager.FlushBlock(nFrames);
}
