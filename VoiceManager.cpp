#include "VoiceManager.h"

VoiceManager::VoiceManager()
{
  // Init note arrays
  for (int i = 0; i < NUM_MIDI_NOTES; i++) {
    mOrderPressed[i] = 0;
    mNoteStatus[i] = kNoteInactive;
  }
}

void VoiceManager::ProcessMidiMessage(const IMidiMsg& msg)
{
  mMidiQueue.Add(msg);
}

void VoiceManager::FlushBlock(int nFrames)
{
  mMidiQueue.Flush(nFrames);
  mSampleOffset -= nFrames;
}

VoiceState* VoiceManager::AdvanceFrame()
{

  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    // First, clear out and clean up last frame's events
    switch (mVoiceStates[i].event)
    {
    case kNoteStart:
    case kNoteRelease:
      // Reset the event so the note can continue
      mVoiceStates[i].event = kNullEvent;
      break;
    case kNoteEnd:
      // The note has ended; reset the voice
      InactivateVoice(i);
      break;
    case kNullEvent:
      // No action necessary
      break;
    }
  }

  // Check for voices that are ready to end and send the event
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (mVoiceStates[i].isReadyToEnd)
    {
      mVoiceStates[i].event = kNoteEnd;
    }
  }

  // Check for currently releasing notes and reset the end flag
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (mVoiceStates[i].nFramesSinceRelease >= 0 && !mVoiceStates[i].isReadyToEnd)
    {
      mVoiceStates[i].nFramesSinceRelease++;
      mVoiceStates[i].isReadyToEnd = true;
    }
  }

  // Check for new midi messages
  // Note that there may be more than one message in a single frame
  while (!mMidiQueue.Empty() && mMidiQueue.Peek().mOffset <= mSampleOffset)
  {
    IMidiMsg message = mMidiQueue.Peek();
    mMidiQueue.Remove();

    IMidiMsg::EStatusMsg status = message.StatusMsg();
    int note = message.NoteNumber();
    int velocity = message.Velocity();

    if (status == IMidiMsg::kNoteOn && velocity > 0)
    {
      HandleNoteOn(note);
    }
    else if (status == IMidiMsg::kNoteOff || (status == IMidiMsg::kNoteOn && velocity == 0))
    {
      // Some midi controllers may set velocity to zero instead of actually turning the note off
      HandleNoteOff(note);
    }

    // TODO pedal magic here

  }

  // Update the offset for the next sample
  mSampleOffset++;

  // Return the array of voices for processing
  return mVoiceStates;
}

void VoiceManager::HandleNoteOn(int note)
{
  // Mark the note as pressed
  mNoteStatus[note] = kNotePressed;

  // Shift the previous notes back one position
  for (int i = 0; i < NUM_MIDI_NOTES; i++) {
    if (mOrderPressed[i] > 0) {
      mOrderPressed[i]++;
    }
  }
  // Insert this note at the front
  mOrderPressed[note] = 1;

  // Start the note
  int voice = FindAllocatableVoice(note);
  ActivateVoice(voice, note);
}

void VoiceManager::HandleNoteOff(int note)
{
  // Mark the note as unpressed
  mNoteStatus[note] = kNoteInactive;
  
  // TODO pedal magic here

  // Move all the notes behind the released note foward one position
  int releaseIndex = mOrderPressed[note];
  mOrderPressed[note] = 0;
  for (int i = 0; i < NUM_MIDI_NOTES; i++) {
    if (mOrderPressed[i] > releaseIndex) {
      mOrderPressed[i] -= 1;
    }
  }

  // Attempt to reassign the voice to another key
  int recovery = FindRecoveryNote();

  // Find the appropriate voice and either send a release event or send the recovery note
  for (int i = 0; i < MAX_NUM_VOICES; i++)
  {
    if (mVoiceStates[i].note == note) {
      if (recovery == -1) {
        // No recovery, send the release event
        mVoiceStates[i].event = kNoteRelease;
        mVoiceStates[i].nFramesSinceRelease = 0;
        mVoiceStates[i].isReadyToEnd = true;
      }
      else
      {
        // Recovery found
        ActivateVoice(i, recovery);
      }
      break;
    }
  }
}

void VoiceManager::HandlePedalOff()
{

  // TODO pedal magic here

}

int VoiceManager::FindAllocatableVoice(int note)
{
  // There are four possible cases here
  // 1. Best case scenario: the note is still releasing and we keep the same voice
  for (int i = 0; i < mNumVoices; i++)
  {
    if (mVoiceStates[i].note == note)
    {
      return i;
    }
  }

  // 2. Look for an inactive voice to assign
  for (int i = 0; i < mNumVoices; i++)
  {
    if (!mVoiceStates[i].isSounding)
    {
      return i;
    }
  }

  // 3. If any notes are currently releasing, find the oldest
  int maxReleaseTime = -1;
  int maxIndex = -1;
  for (int i = 0; i < mNumVoices; i++) {
    if (mVoiceStates[i].nFramesSinceRelease > maxReleaseTime)
    {
      maxReleaseTime = mVoiceStates[i].nFramesSinceRelease;
      maxIndex = i;
    }
  }
  if (maxIndex != -1)
  {
    return maxIndex;
  }

  // 4. All voices are in use; recycle the oldest
  int maxStackPosition = -1;
  int oldestVoice = -1;
  for (int i = 0; i < mNumVoices; i++) {
    int note = mVoiceStates[i].note;
    int stackPos = mOrderPressed[note];
    if (stackPos > maxStackPosition)
    {
      maxStackPosition = stackPos;
      oldestVoice = i;
    }
  }
  return oldestVoice;
}

int VoiceManager::FindRecoveryNote()
{
  int minStackPos = NUM_MIDI_NOTES;
  int note = -1;
  for (int i = 0; i < NUM_MIDI_NOTES; i++) {
    if (mOrderPressed[i] > 0 && mOrderPressed[i] < minStackPos) {
      // Make sure the note isn't already sounding
      bool isSounding = false;
      for (int j = 0; j < MAX_NUM_VOICES; j++)
      {
        if (mVoiceStates[j].note == i)
        {
          isSounding = true;
          break;
        }
      }
      if (!isSounding)
      {
        minStackPos = mOrderPressed[i];
        note = i;
      }
    }
  }
  // Note: -1 is a possible return value if nothing is found. This is intentional.
  return note;
}

void VoiceManager::ActivateVoice(int voice, int note)
{
  mVoiceStates[voice].isSounding = true;
  mVoiceStates[voice].event = kNoteStart;
  mVoiceStates[voice].note = note;
  // leave frequency and sample calculations to the respective parts
  mVoiceStates[voice].nFramesSinceRelease = -1;
  mVoiceStates[voice].isReadyToEnd = false;
}

void VoiceManager::InactivateVoice(int voice)
{
  mVoiceStates[voice].isSounding = false;
  mVoiceStates[voice].event = kNullEvent;
  mVoiceStates[voice].note = -1;
  mVoiceStates[voice].frequency = 0;
  mVoiceStates[voice].sampleValue = 0;
  mVoiceStates[voice].nFramesSinceRelease = -1;
  mVoiceStates[voice].isReadyToEnd = false;
}
