#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IPlugMidi.h"

#define NUM_MIDI_NOTES 128

#define MAX_NUM_VOICES 4

using namespace iplug;

enum EVoiceEvent
{
  kNullEvent, // No event this frame
  kNoteStart, // A new note is sounding this frame
  kNoteRelease, // A note is released this frame
  kNoteEnd, // A note has finished sounding this frame
};

enum EMidiNoteStatus
{
  kNoteInactive, // when a note is neither pressed nor pedaled
  kNotePressed, // when the key is currently down
  kNotePedaled, // when the note is sustained only via pedal
};

struct VoiceState
{
  bool isSounding = false; // True if the voice is making any sound
  EVoiceEvent event = kNullEvent; // An event that is happening this frame

  int note = -1; // The midi key this voice is assigned to
  double frequency = 0.0; // The frequency this voice is currently sounding at
  double sampleValue = 0.0; // The sample value contributed by this voice

  int nFramesSinceRelease = -1; // Used for reallocating voices
  bool isReadyToEnd = false; // For released notes that aren't finished sounding
};

class VoiceManager
{
public:
  VoiceManager();

  void ProcessMidiMessage(const IMidiMsg& msg);
  void FlushBlock(int nFrames);
  VoiceState* AdvanceFrame();

private:
  IMidiQueue mMidiQueue;
  VoiceState mVoiceStates[MAX_NUM_VOICES];

  int mOrderPressed[NUM_MIDI_NOTES];
  EMidiNoteStatus mNoteStatus[NUM_MIDI_NOTES];

  int mNumVoices = 4;

  bool mIsPedalOn = false;

  int mSampleOffset = 0;

  void HandleNoteOn(int note);
  void HandleNoteOff(int note);
  void HandlePedalOff();

  int FindAllocatableVoice(int note);
  int FindRecoveryNote();

  void ActivateVoice(int voice, int note);
  void InactivateVoice(int voice);
};
