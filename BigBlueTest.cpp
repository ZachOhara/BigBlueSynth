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
  // Init the interface manager
  mBBInterfaceManager.SetDelegate(this);

  // Init modules
  // --------------------
  RegisterModule(&mTuningProc);
  RegisterModule(&mPortamentoProcessor);
  RegisterModule(&mOscillator1);
  RegisterModule(&mOscillator2);
  RegisterModule(&mOscMixer);
  mOscMixer.AddOscillator(&mOscillator1);
  mOscMixer.AddOscillator(&mOscillator2);
  RegisterModule(&mEnvelopeProcessor);
  RegisterModule(&mFilterProcessor);
  RegisterModule(&mPitchWheelProcessor);
  // Init parameters
  // --------------------
  // Synth mode
  GetParam(kSynthMode)->InitEnum("Synth Mode", ESynthMode::kSynthModePoly, SYNTH_MODE_NAMES);
  GetParam(kActiveVoices)->InitInt("Voices", 8, 1, MAX_NUM_VOICES, "");
  // Portamento
  GetParam(kPortamentoMode)->InitEnum("Portamento Mode", EPortamentoMode::kPortamentoModeNever, PORTAMENTO_MODE_NAMES);
  GetParam(kPortamentoType)->InitEnum("Portamento Type", EPortamentoType::kPortamentoTypeTime, PORTAMENTO_TYPE_NAMES);
  GetParam(kPortamentoTime)->InitDouble("Portamento Time", 100, 0, 5000, 1, "ms", 0, "", IParam::ShapePowCurve(3.0));
  GetParam(kPortamentoRate)->InitDouble("Portamento Rate", 30, 0, 500, 1, "ms/st", 0, "", IParam::ShapePowCurve(2.5));
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
  GetParam(kEnvAttackPid)->InitDouble("Envelope Attack Time", 0, 0, 5000, 1, "ms", 0, "", IParam::ShapePowCurve(2.0));
  GetParam(kEnvDecayPid)->InitDouble("Envelope Decay Time", 0, 0, 5000, 1, "ms", 0, "", IParam::ShapePowCurve(2.0));
  GetParam(kEnvSustainPid)->InitDouble("Envelope Sustain Level", 100.0, 0.0, 100.0, 0.1, "%");
  GetParam(kEnvReleasePid)->InitDouble("Envelope Release Time", 0, 0, 5000, 1, "ms", 0, "", IParam::ShapePowCurve(2.0));
  GetParam(kEnvPeakPid)->InitDouble("Envelope Peak Level", 100.0, 0.0, 100.0, 0.1, "%");
  // Filter
  GetParam(kFilCutoffPid)->InitDouble("Filter Cutoff Frequency", 22000.0, 0.0, 22000.0, 0.1, "hz", 0, "", IParam::ShapePowCurve(2.0));
  // --------------------
  // Init interface
  mLayoutFunc = [&](IGraphics* pGraphics) {
    mBBInterfaceManager.LayoutFunction(pGraphics);
  };
}

void BigBlueTest::OnParamChange(int pid)
{
  // Pass update notification to the interface manager
  mBBInterfaceManager.NotifyParamChange(pid);
  // Handle parameter for the audio thread
  switch (pid)
  {
    // Synth mode
    // ---------------------
  case kSynthMode:
    if (GetParam(pid)->Int() == kSynthModePoly)
    {
      mVoiceManager.SetActiveVoices(GetParam(kActiveVoices)->Int());
    }
    else
    {
      mVoiceManager.SetActiveVoices(1);
    }
    break;
  case kActiveVoices:
    mVoiceManager.SetActiveVoices(GetParam(pid)->Int());
    break;
    // Portamento
    // ---------------------
  case kPortamentoMode:
    mPortamentoProcessor.SetPortamentoMode((EPortamentoMode)GetParam(pid)->Int());
    break;
  case kPortamentoType:
    mPortamentoProcessor.SetPortamentoType((EPortamentoType)GetParam(pid)->Int());
    break;
  case kPortamentoTime:
    mPortamentoProcessor.SetPortamentoTime(GetParam(pid)->Value() / 1000.0);
    break;
  case kPortamentoRate:
    mPortamentoProcessor.SetPortamentoRate(GetParam(pid)->Value() / 1000.0);
    break;
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
  // Filter Processor
  // ---------------------
  case kFilCutoffPid:
    mFilterProcessor.SetCutoffFrequency(GetParam(pid)->Value());
    break;
  default:
    break;
  }
}

void BigBlueTest::OnIdle()
{
  mBBInterfaceManager.ProcessQueuedUpdates();
}

void BigBlueTest::ProcessMidiMsg(const IMidiMsg& message)
{
  IMidiMsg::EStatusMsg status = message.StatusMsg();
  bool isPedalMsg = (status == IMidiMsg::kControlChange
    && message.ControlChangeIdx() == IMidiMsg::kSustainOnOff);
  // If this is a message about a note (or the sustain pedal), send it to the voice manager
  if (status == IMidiMsg::kNoteOn || status == IMidiMsg::kNoteOff || isPedalMsg)
  {
    mVoiceManager.ProcessMidiMessage(message);
  }
  else
  {
    mSysMidiQueue.Add(message);
  }
}

void BigBlueTest::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = 0.7; // GetParam(kOsc1WaveformPid)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {

    ProcessSystemMessages(s);

    VoiceState* voices = mVoiceManager.AdvanceFrame();

    // Process the set of voices through each module in order
    mTuningProc.ProcessVoices(voices);
    mPortamentoProcessor.ProcessVoices(voices);
    mPitchWheelProcessor.ProcessVoices(voices);
    mOscillator1.ProcessVoices(voices);
    mOscillator2.ProcessVoices(voices);
    mOscMixer.ProcessVoices(voices);
    mEnvelopeProcessor.ProcessVoices(voices);
    mFilterProcessor.ProcessVoices(voices);

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
  mSysMidiQueue.Flush(nFrames);
}

void BigBlueTest::ProcessSystemMessages(int sampleOffset)
{
  // Check for new midi messages
  // Note that there may be more than one message in a single frame
  while (!mSysMidiQueue.Empty() && mSysMidiQueue.Peek().mOffset <= sampleOffset)
  {
    IMidiMsg message = mSysMidiQueue.Peek();
    mSysMidiQueue.Remove();

    IMidiMsg::EStatusMsg status = message.StatusMsg();


    if (status == IMidiMsg::kPitchWheel)
    {
      mPitchWheelProcessor.SetWheelPosition(message.PitchWheel());
    }
  }

}
