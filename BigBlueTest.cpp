#include "BigBlueTest.h"
#include "IPlug_include_in_plug_src.h"

BigBluePlugin::BigBluePlugin(const InstanceInfo& info, int nParams, int nPresets)
  : Plugin(info, MakeConfig(nParams, nPresets))
{
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
  };
}

BigBlueTest::BigBlueTest(const InstanceInfo& info)
  : BigBluePlugin(info, kNumParams, kNumPresets)
{
  // Init parameters
  GetParam(kOsc1OctavePid)->InitInt("Osc 1 Octave", 0, -1, 2, "Osc 1 Octave");
  GetParam(kOsc1WaveformPid)->InitEnum("Osc 1 Waveform", EWaveform::kSineWave, WAVEFORM_NAMES);

  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachPanelBackground(COLOR_MAT_BGRAY900);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello iPlug 2!", IText(20, COLOR_WHITE)));
    pGraphics->AttachControl(new BBKnobControl(b.GetCentredInside(100).GetVShifted(-100), kOsc1WaveformPid, "Waveform"));
    auto box = b.GetCentredInside(50, 80).GetVShifted(100);
    pGraphics->AttachControl(new BBSlideSelectControl(pGraphics, box, kOsc1OctavePid, OCTAVE_NAMES, "Octave", true));
    pGraphics->AttachControl(new BBSlideSelectControl(pGraphics, box.GetHShifted(60), kOsc1WaveformPid, WAVEFORM_NAMES, "Waveform", false));
    

  };
}

void BigBlueTest::OnParamChange(int pid)
{
  switch (pid)
  {
  case kOsc1OctavePid:
    mOscillator.SetOctaveMod(GetParam(pid)->Int());
    break;
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
