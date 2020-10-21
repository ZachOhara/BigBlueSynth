#include "BigBlueTest.h"
#include "IPlug_include_in_plug_src.h"

///*
BigBluePlugin::BigBluePlugin(const InstanceInfo& info, int nParams, int nPresets)
  : Plugin(info, MakeConfig(nParams, nPresets))
{
}
//*/

BigBlueTest::BigBlueTest(const InstanceInfo& info)
  : BigBluePlugin(info, kNumParams, kNumPresets)
//  : Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  //GetParam(kOsc1WaveformPid)->InitDouble("Osc 1 Waveform", 0., 0., 100.0, 0.01, "%");

  GetParam(kOsc1WaveformPid)->InitEnum("Osc 1 Waveform", EWaveform::kSineWave, { "Sine", "Triangle", "Square", "Saw" });

  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachPanelBackground(COLOR_MAT_BGRAY900);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello iPlug 2!", IText(20, COLOR_WHITE)));
    pGraphics->AttachControl(new BigBlueKnobControl(b.GetCentredInside(100).GetVShifted(-100), kOsc1WaveformPid, "Waveform"));
    auto box = b.GetCentredInside(100).GetVShifted(100);
    //auto box = b.GetCentredInside(50, 100).GetVShifted(100);
    //pGraphics->AttachControl(new IVSwitchControl(b.GetCentredInside(100).GetVShifted(100), kOsc1WaveformPid, "Waveform"));
    //pGraphics->AttachControl(new IPopupMenuControl(kOsc1WaveformPid, IText(12, "Roboto-Regular"), b.GetCentredInside(100).GetVShifted(100), b.GetCentredInside(100).GetVShifted(100)));
    //pGraphics->AttachControl(new IVSliderControl(box, kOsc1WaveformPid, "Wavefm", BigBlueHouseStyle));
    //pGraphics->AttachControl(new IVSlideSwitchControl(box, kOsc1WaveformPid, "Wavefm", BigBlueHouseStyle, true, EDirection::Vertical));
    pGraphics->AttachControl(new BigBlueSelectSliderControl(pGraphics, box, kOsc1WaveformPid, { "Sine", "Triangle", "Square", "Saw" }, "Wavefm"));

  };
}

void BigBlueTest::OnParamChange(int pid)
{
  switch (pid)
  {
  case kOsc1WaveformPid:
    mOscillator.SetWaveform((EWaveform)GetParam(pid)->Int());
    break;
  default:
    break;
  }
}

void BigBlueTest::OnReset()
{
  //mOscillator.HandleHostReset();
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
