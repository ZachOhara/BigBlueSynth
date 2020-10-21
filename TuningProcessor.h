#pragma once

#include "VoiceManager.h"

#include <cmath>

#define NUM_KEYS 12

class TuningProcessor
{
public:
  //enum EParameters { kTemperamentParam, kKeyParam, kNumParams };
  //enum ETemperaments { kEqualTemperament, kJustTemperamentMajor, kJustTemperamentMinor };

  TuningProcessor();
  ~TuningProcessor();

  void ProcessVoices(VoiceState* voices);

  //void SetTemperament(int temperament);
  //void SetKey(int key);

private:
  const int TUNING_A_ID = 57;
  const double TUNING_A_HZ = 440.0;

  /*
  const int justifiedMajorRatios[12][2] = {
    {1, 1}, {16, 15}, {9, 8}, {6, 5}, {5, 4}, {4, 3},
    {45, 32}, {3, 2}, {8, 5}, {5, 3}, {9, 5}, {15, 8}
  };
  */

  double equalTemperamentPitches[NUM_MIDI_NOTES];

  //double justTemperamentTunings[NUM_KEYS][NUM_MIDI_NOTES];

  int mTuningMode;
  int mCurrentKey;
  int mRelativeMajorKey;

  double GetFrequency(int note);

  void InitEqualTemperamentTunings();

  //void InitJustTemperamentTunings();
};

