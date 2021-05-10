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

BigBlueTest::BigBlueTest(const InstanceInfo& info) :
  BigBluePlugin(info, kNumParams, kNumPresets),
  mOscMixer(2)
{
  // Init modules
  // --------------------
  RegisterModule(&mTuningProc);
  RegisterModule(&mOscillator1);
  RegisterModule(&mOscillator2);
  RegisterModule(&mOscMixer);
  mOscMixer.AddOscillator(&mOscillator1);
  mOscMixer.AddOscillator(&mOscillator2);
  RegisterModule(&mEnvelopeProcessor);
  // Init parameters
  // --------------------
  // Oscillator 1
  GetParam(kOsc1OctavePid)->InitInt("Osc 1 Octave", 0, -1, 2, "", IParam::kFlagSignDisplay);
  GetParam(kOsc1OctavePid)->SetDisplayText(0, "+0");
  GetParam(kOsc1WaveformPid)->InitEnum("Osc 1 Waveform", EWaveform::kSineWave, WAVEFORM_NAMES);
  GetParam(kOsc1SemitonePid)->InitInt("Osc 1 Semitone", 0, -12, 12, "st", IParam::kFlagSignDisplay);
  GetParam(kOsc1SemitonePid)->SetDisplayFunc(&SignedDisplayFunc);
  GetParam(kOsc1DetunePid)->InitDouble("Osc 1 Detune", 0, -100, 100, 0.01, "c", IParam::kFlagSignDisplay);
  GetParam(kOsc1DetunePid)->SetDisplayText(0, "+0.00 c");
  // Oscillator 2
  GetParam(kOsc2OctavePid)->Init(*GetParam(kOsc1OctavePid), "1", "2");
  GetParam(kOsc2WaveformPid)->Init(*GetParam(kOsc1WaveformPid), "1", "2");
  GetParam(kOsc2SemitonePid)->Init(*GetParam(kOsc1SemitonePid), "1", "2");
  GetParam(kOsc2DetunePid)->Init(*GetParam(kOsc1DetunePid), "1", "2");
  // Mixer
  GetParam(kMixLevelOsc1)->InitDouble("Osc 1 Mix Level", 0.5, 0.0, 1.0, 0.001);
  GetParam(kMixLevelOsc2)->Init(*GetParam(kMixLevelOsc1), "1", "2");
  // Envelope
  GetParam(kEnvAttackPid)->InitDouble("Envelope Attack Time", 0, 0, 5000, 1, "ms", 0, "", IParam::ShapePowCurve(3.0));
  GetParam(kEnvDecayPid)->InitDouble("Envelope Decay Time", 0, 0, 5000, 1, "ms", 0, "", IParam::ShapePowCurve(3.0));
  GetParam(kEnvSustainPid)->InitDouble("Envelope Sustain Level", 100.0, 0.0, 100.0, 0.1, "%");
  GetParam(kEnvReleasePid)->InitDouble("Envelope Release Time", 0, 0, 5000, 1, "ms", 0, "", IParam::ShapePowCurve(3.0));
  // Init interface
  // --------------------
  mLayoutFunc = [&](IGraphics* pGraphics) {
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
    pGraphics->AttachControl(new BBSliderControl(envelopeBox.GetHShifted(h+0).GetHSliced(40), kEnvAttackPid, "Attack", BB_DEFAULT_ACCENT_COLOR, 9.f, 4.f));
    pGraphics->AttachControl(new BBSliderControl(envelopeBox.GetHShifted(h+45).GetHSliced(40), kEnvDecayPid, "Decay", BB_DEFAULT_ACCENT_COLOR, 9.f, 4.f));
    pGraphics->AttachControl(new BBSliderControl(envelopeBox.GetHShifted(h+90).GetHSliced(40), kEnvSustainPid, "Sustain", BB_DEFAULT_ACCENT_COLOR, 9.f, 4.f));
    pGraphics->AttachControl(new BBSliderControl(envelopeBox.GetHShifted(h+135).GetHSliced(40), kEnvReleasePid, "Release", BB_DEFAULT_ACCENT_COLOR, 9.f, 4.f));

   };
}

void BigBlueTest::OnParamChange(int pid)
{
  switch (pid)
  {
    // Oscillator 1
    // ---------------------
  case kOsc1OctavePid:
    mOscillator1.SetOctaveMod(GetParam(pid)->Value());
    break;
  case kOsc1WaveformPid:
    mOscillator1.SetWaveform((EWaveform)GetParam(pid)->Int());
    break;
  case kOsc1SemitonePid:
    mOscillator1.SetSemitoneMod(GetParam(pid)->Value());
    break;
  case kOsc1DetunePid:
    mOscillator1.SetCentsMod(GetParam(pid)->Value());
    break;
    // Oscillator 2
    // ---------------------
  case kOsc2OctavePid:
    mOscillator2.SetOctaveMod(GetParam(pid)->Value());
    break;
  case kOsc2WaveformPid:
    mOscillator2.SetWaveform((EWaveform)GetParam(pid)->Int());
    break;
  case kOsc2SemitonePid:
    mOscillator2.SetSemitoneMod(GetParam(pid)->Value());
    break;
  case kOsc2DetunePid:
    mOscillator2.SetCentsMod(GetParam(pid)->Value());
    break;
    // Osc Mixer
    // ---------------------
  case kMixLevelOsc1:
    mOscMixer.SetMixLevel(0, GetParam(pid)->Value());
    break;
  case kMixLevelOsc2:
    mOscMixer.SetMixLevel(1, GetParam(pid)->Value());
    break;
  // Envelope Processor
  // ---------------------
  case kEnvAttackPid:
    mEnvelopeProcessor.SetAttackTime(GetParam(pid)->Value());
    break;
  case kEnvDecayPid:
    mEnvelopeProcessor.SetDecayTime(GetParam(pid)->Value());
    break;
  case kEnvSustainPid:
    mEnvelopeProcessor.SetSustainLevel(GetParam(pid)->Value() / 100.0);
    break;
  case kEnvReleasePid:
    mEnvelopeProcessor.SetReleaseTime(GetParam(pid)->Value());
    break;
  case kEnvPeakPid:
    mEnvelopeProcessor.SetPeakLevel(GetParam(pid)->Value() / 100.0);
    break;
  default:
    break;
  }
}

void BigBlueTest::ProcessMidiMsg(const IMidiMsg& msg)
{

  //if (msg.StatusMsg() == IMidiMsg::kNoteOn) {
  //  mOscillator1.SetFrequency(mTuningProc.GetFrequency(msg.NoteNumber()));
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
    mOscillator1.ProcessVoices(voices);
    mOscillator2.ProcessVoices(voices);
    mOscMixer.ProcessVoices(voices);
    //mEnvelopeProcessor.ProcessVoices(voices);

    // Combine the voices into a sample value
    double sample = 0;
    for (int i = 0; i < MAX_NUM_VOICES; i++) {
      sample += voices[i].sampleValue;
    }

    // This is an adequate way to prevent clipping
    sample /= MAX_NUM_VOICES;
    sample /= 2;

    // Assign the signal to each channel
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = sample * gain;
    }
  }

  mVoiceManager.FlushBlock(nFrames);
}
